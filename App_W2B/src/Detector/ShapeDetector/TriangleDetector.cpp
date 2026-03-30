#include "TriangleDetector.hpp"
#include "../GeometryUtils.hpp"
#include <algorithm>
#include <cmath>
#include <array>

float TriangleDetector::getContourAngle(const std::vector<cv::Point> &contour) // Override the default implementation to get the angle of a contour
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

double TriangleDetector::vertexAngleDeg(const cv::Point &a, const cv::Point &b, const cv::Point &c) // Calculate the angle at vertex b formed by points a and c in degrees
{
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

std::vector<Shape> TriangleDetector::detect(const cv::Mat &processedImage, const cv::Mat &originalImage) // Override the default implementation to detect triangles in the processed image and return their information as a vector of Shape objects
{
    std::vector<Shape> found;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<cv::Rect> acceptedRects;
    cv::findContours(processedImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    const double minArea = 120.0;
    const double sideRatioMin = 0.2;
    const double minVertexAngle = 4.0;

    for (size_t i = 0; i < contours.size(); ++i) // Iterate through each contour and apply various checks to determine if it is a valid triangle, 
    // and if so, extract its properties and add it to the list of found shapes
    {
        std::vector<cv::Point> approx;
        double peri = cv::arcLength(contours[i], true);
        double eps = std::max(2.0, 0.05 * peri);
        cv::approxPolyDP(contours[i], approx, eps, true);

        if (approx.size() != 3)
            continue;

        if (!cv::isContourConvex(approx))
            continue;

        double area = std::abs(cv::contourArea(approx));
        if (area < minArea) // Filter out small triangles based on area
            continue;

        std::array<double, 3> sides = {{
            static_cast<double>(cv::norm(approx[0] - approx[1])),
            static_cast<double>(cv::norm(approx[1] - approx[2])),
            static_cast<double>(cv::norm(approx[2] - approx[0]))}};
        double minS = *std::min_element(sides.begin(), sides.end());
        double maxS = *std::max_element(sides.begin(), sides.end());
        if (maxS == 0.0 || minS < sideRatioMin * maxS) // Filter out triangles that are too elongated based on the ratio of the shortest to longest side
            continue;

        double a0 = this->vertexAngleDeg(approx[2], approx[0], approx[1]);
        double a1 = this->vertexAngleDeg(approx[0], approx[1], approx[2]);
        double a2 = this->vertexAngleDeg(approx[1], approx[2], approx[0]);
        double minAngle = std::min({a0, a1, a2});
        if (minAngle < minVertexAngle) // Filter out triangles that have a very small angle, which may indicate that they are not well-defined triangles
            continue;

        cv::Rect rect = cv::boundingRect(approx); // Get the bounding rectangle of the triangle
        if (rect.width <= 25 || rect.height <= 25) // Filter out triangles that are too small based on their bounding rectangle
            continue;

        bool isDuplicate = false;
        for (const auto &accepted : acceptedRects)
        {
            if (GeometryUtils::rectIntersectionOverUnion(rect, accepted) > 0.70)
            {
                isDuplicate = true;
                break;
            }
        }

        if (isDuplicate)
            continue;

        float angle = this->getContourAngle(approx);
        cv::Scalar avgBgr = this->getContourColor(approx, originalImage);

        Shape shape;
        shape.type = "triangle";
        shape.x = rect.x;
        shape.y = rect.y;
        shape.width = rect.width;
        shape.height = rect.height;
        shape.color = avgBgr;
        shape.angle = angle;
        found.push_back(shape);
        acceptedRects.push_back(rect);
    }
    return found;
}