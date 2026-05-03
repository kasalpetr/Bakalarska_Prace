#include "SquareDetector.hpp"
#include "../GeometryUtils.hpp"
#include <algorithm>
#include <cmath>

double SquareDetector::vertexAngleDeg(const cv::Point &a, const cv::Point &b, const cv::Point &c)
{
    cv::Point2d v1 = cv::Point2d(a - b);
    cv::Point2d v2 = cv::Point2d(c - b);
    double dot = v1.x * v2.x + v1.y * v2.y;
    double n1 = std::hypot(v1.x, v1.y);
    double n2 = std::hypot(v2.x, v2.y);
    if (n1 == 0.0 || n2 == 0.0)
    {
        return 0.0;
    }

    double cosv = dot / (n1 * n2);
    cosv = std::max(-1.0, std::min(1.0, cosv));
    return std::acos(cosv) * 180.0 / CV_PI;
}

bool SquareDetector::hasMostlyRightAngles(const std::vector<cv::Point> &quad)
{
    if (quad.size() != 4)
    {
        return false;
    }

    for (int i = 0; i < 4; ++i)
    {
        const cv::Point &prev = quad[(i + 3) % 4];
        const cv::Point &current = quad[i];
        const cv::Point &next = quad[(i + 1) % 4];
        double angle = this->vertexAngleDeg(prev, current, next);
        if (angle < 60.0 || angle > 120.0)
        {
            return false;
        }
    }

    return true;
}

bool SquareDetector::isInnerSameCollor(const std::vector<cv::Point> &contour, const cv::Mat &originalImage) const // Check if the interior of the contour has a similar color like border
{
    if (contour.size() < 3 || originalImage.empty())
    {
        return false;
    }

    cv::Rect rect = cv::boundingRect(contour);
    int minDim = std::min(rect.width, rect.height);
    if (minDim < 12)
    {
        return false;
    }

    cv::Mat filledMask = cv::Mat::zeros(originalImage.size(), CV_8UC1);
    cv::drawContours(filledMask, std::vector<std::vector<cv::Point>>{contour}, 0, cv::Scalar(255), cv::FILLED);

    int erodeRadius = std::max(1, static_cast<int>(std::round(minDim * 0.08)));
    cv::Mat erodeKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                                    cv::Size(2 * erodeRadius + 1, 2 * erodeRadius + 1));

    cv::Mat innerMask;
    cv::erode(filledMask, innerMask, erodeKernel);

    cv::Mat borderMask;
    cv::subtract(filledMask, innerMask, borderMask);

    int innerPixels = cv::countNonZero(innerMask);
    int borderPixels = cv::countNonZero(borderMask);
    if (innerPixels < 40 || borderPixels < 20) 
    {
        return false;
    }

    // If interior is mostly whiteboard-like (white or near-white), do not classify as sticky note.
    cv::Mat hsvImage;
    cv::cvtColor(originalImage, hsvImage, cv::COLOR_BGR2HSV);

    cv::Mat whiteLikeMask;
    cv::inRange(hsvImage, cv::Scalar(0, 0, 170), cv::Scalar(179, 70, 255), whiteLikeMask);

    cv::Mat whiteInnerMask;
    cv::bitwise_and(whiteLikeMask, innerMask, whiteInnerMask);

    int whiteInnerPixels = cv::countNonZero(whiteInnerMask);
    double whiteRatio = static_cast<double>(whiteInnerPixels) / static_cast<double>(innerPixels);
    if (whiteRatio >= 0.45)
    {
        return false;
    }

    cv::Scalar borderMean = cv::mean(originalImage, borderMask);
    cv::Scalar innerMean  = cv::mean(originalImage, innerMask);

    double colorDist = std::sqrt(
        std::pow(borderMean[0] - innerMean[0], 2.0) +
        std::pow(borderMean[1] - innerMean[1], 2.0) +
        std::pow(borderMean[2] - innerMean[2], 2.0));

    return colorDist <= 40.0;
}

float SquareDetector::getContourAngle(const std::vector<cv::Point> &contour) // Override the default implementation to get the angle of a contour
{
    cv::RotatedRect rotatedRect = cv::minAreaRect(contour);
    float angle = rotatedRect.angle;
    cv::Size2f rectSize = rotatedRect.size;

    if (rectSize.width < rectSize.height)
    {
        angle += 90.0f;
    }
    return angle;
}

std::vector<Shape> SquareDetector::detect(const cv::Mat &processedImage, const cv::Mat &originalImage) // Override the default implementation to detect squares in the processed image and return their information as a vector of Shape objects
{
    std::vector<Shape> found;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<cv::Rect> acceptedRects;
    cv::findContours(processedImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    for (size_t i = 0; i < contours.size(); ++i)
    {
        std::vector<cv::Point> approx;
        double peri = cv::arcLength(contours[i], true);
        cv::approxPolyDP(contours[i], approx, 0.04 * peri, true);

        if (approx.size() >= 4 && approx.size() <= 6) // Filter contours that are not quadrilaterals (squares or rectangles)
        {
            if (!cv::isContourConvex(approx))
            {
                continue;
            }

            if (approx.size() == 4 && !this->hasMostlyRightAngles(approx)) 
            {
                continue;
            }

            double area = std::abs(cv::contourArea(approx)); 
            if (area < 500.0)
            {
                continue;
            }

            double perimeter = cv::arcLength(approx, true);
            if (perimeter <= 0.0)
            {
                continue;
            }

            double circularity = 4.0 * CV_PI * area / (perimeter * perimeter);
            if (circularity > 0.82)
            {
                continue;
            }

            cv::RotatedRect rotatedRect = cv::minAreaRect(approx);
            double rotatedArea = static_cast<double>(rotatedRect.size.width) * rotatedRect.size.height;
            if (rotatedArea <= 0.0)
            {
                continue;
            }

            double fillRatio = area / rotatedArea;
            if (fillRatio < 0.45)
            {
                continue;
            }

            cv::Rect rect = cv::boundingRect(approx); 
            if (rect.width > 25 && rect.height > 25)
            {
                bool isDuplicate = false;
                for (const auto &accepted : acceptedRects)
                {
                    double intersectOfRectangle = GeometryUtils::rectIntersectionOverUnion(rect, accepted);
                    if (intersectOfRectangle > 0.75 || (intersectOfRectangle > 0.35 && GeometryUtils::areLikelyDoubleEdgeRectangles(rect, accepted)))
                    {
                        isDuplicate = true;
                        break;
                    }
                }

                if (isDuplicate)
                {
                    continue;
                }

                float angle = this->getContourAngle(approx);
                cv::Scalar avgBgr = this->getContourColor(approx, originalImage);

                std::string shapeType = this->isInnerSameCollor(approx, originalImage) ? "stickyNote" : "rectangle";
                Shape shape;
                shape.type = shapeType;
                shape.x = rect.x;
                shape.y = rect.y;
                shape.width = rect.width;
                shape.height = rect.height;
                shape.color = avgBgr;
                shape.angle = angle;
                found.push_back(shape);
                acceptedRects.push_back(rect); 
            }
        }
    }
    return found;
}