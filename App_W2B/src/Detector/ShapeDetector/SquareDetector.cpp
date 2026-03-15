#include "SquareDetector.hpp"
#include "../GeometryUtils.hpp"
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

        if (approx.size() >= 4 && approx.size() <= 6)
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
                    if (GeometryUtils::rectIntersectionOverUnion(rect, accepted) > 0.75)
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

                found.push_back({"rectangle", rect.x, rect.y, rect.width, rect.height, avgBgr, angle});
                acceptedRects.push_back(rect);
            }
        }
    }
    return found;
}