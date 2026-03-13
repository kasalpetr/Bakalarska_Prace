#include "ShapeDetector.hpp"
#include <algorithm>
#include <cmath>
#include <array>

class TriangleDetector : public ShapeDetector
{ // specific implementation for triangle detection
protected:
    float getContourAngle(const std::vector<cv::Point> &contour) override
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

    static double vertexAngleDeg(const cv::Point &a, const cv::Point &b, const cv::Point &c)
    {
        // angle at b between BA and BC
        cv::Point2d v1 = cv::Point2d(a - b);
        cv::Point2d v2 = cv::Point2d(c - b);
        double dot = v1.x * v2.x + v1.y * v2.y;
        double n1 = std::hypot(v1.x, v1.y);
        double n2 = std::hypot(v2.x, v2.y);
        if (n1 == 0.0 || n2 == 0.0)
            return 0.0;
        double cosv = dot / (n1 * n2);
        cosv = std::max(-1.0, std::min(1.0, cosv));
        return std::acos(cosv) * 180.0 / CV_PI;
    }

public:
    std::vector<Shape> detect(const cv::Mat &processedImage, const cv::Mat &originalImage) override
    {
        std::vector<Shape> found;
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(processedImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        const double minArea = 20.0;      // minimal contour area
        const double sideRatioMin = 0.2;  // minSide >= sideRatioMin * maxSide
        const double minVertexAngle = 4.0; // degrees

        for (size_t i = 0; i < contours.size(); ++i)
        {
            std::vector<cv::Point> approx;
            double peri = cv::arcLength(contours[i], true);
            double eps = std::max(2.0, 0.02 * peri); // adaptive epsilon, lower than 0.05
            cv::approxPolyDP(contours[i], approx, eps, true);

            if (approx.size() != 3)
                continue;

            if (!cv::isContourConvex(approx))
                continue;

            double area = std::abs(cv::contourArea(approx));
            if (area < minArea)
                continue;

            // compute side lengths
            std::array<double, 3> sides = {{
                static_cast<double>(cv::norm(approx[0] - approx[1])),
                static_cast<double>(cv::norm(approx[1] - approx[2])),
                static_cast<double>(cv::norm(approx[2] - approx[0]))}};
            double minS = *std::min_element(sides.begin(), sides.end());
            double maxS = *std::max_element(sides.begin(), sides.end());
            if (maxS == 0.0 || minS < sideRatioMin * maxS) // reject very elongated triangles
                continue;

            // check vertex angles to reject nearly-collinear vertices
            double a0 = vertexAngleDeg(approx[2], approx[0], approx[1]);
            double a1 = vertexAngleDeg(approx[0], approx[1], approx[2]);
            double a2 = vertexAngleDeg(approx[1], approx[2], approx[0]);
            double minAngle = std::min({a0, a1, a2});
            if (minAngle < minVertexAngle) // reject if any angle is too small (nearly collinear)
                continue;

            // bounding rect size check (reject tiny detections)
            cv::Rect rect = cv::boundingRect(approx);
            if (rect.width <= 10 || rect.height <= 10)
                continue;

            // improved inner-stroke detection: compare parent contour area similarity
            bool isInnerStroke = false;
            int parentIdx = hierarchy[i][3];
            if (parentIdx != -1)
            {
                double parentArea = std::abs(cv::contourArea(contours[parentIdx]));
                if (parentArea > 0.0 && std::abs(parentArea - area) < 0.2 * parentArea)
                {
                    isInnerStroke = true;
                }
            }

            if (isInnerStroke)
                continue;

            float angle = getContourAngle(approx);
            cv::Scalar avgBgr = getContourColor(approx, originalImage);

            found.push_back({"triangle", rect.x, rect.y, rect.width, rect.height, avgBgr, angle});
        }
        return found;
    }
};