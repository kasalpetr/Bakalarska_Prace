#include "CircleDetector.hpp"
#include "../GeometryUtils.hpp"
#include <cmath>

float CircleDetector::getContourAngle(const std::vector<cv::Point> &contour) // Override the default implementation to get the angle of a contour (for circles, we can just return 0 as they are symmetric)
{
    return 0.0f;
}

std::vector<Shape> CircleDetector::detect(const cv::Mat &processedImage, const cv::Mat &originalImage) // Override the default implementation to detect circles in the processed image and return their information as a vector of Shape objects
{
    std::vector<Shape> found;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<cv::Rect> acceptedRects;

    cv::findContours(processedImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    for (size_t i = 0; i < contours.size(); ++i)
    {
        double area = std::abs(cv::contourArea(contours[i]));
        if (area < 180.0)
            continue;

        double perimeter = cv::arcLength(contours[i], true);
        if (perimeter <= 0.0)
            continue;

        double circularity = 4.0 * CV_PI * area / (perimeter * perimeter);
        if (circularity < 0.62 || circularity > 1.25)
            continue;

        cv::Point2f center;
        float radius = 0.0f;
        cv::minEnclosingCircle(contours[i], center, radius);
        if (radius < 10.0f)
            continue;

        float maxAllowedRadius = static_cast<float>(std::min(originalImage.cols, originalImage.rows)) * 0.20f;
        if (radius > maxAllowedRadius)
            continue;

        cv::Rect rect(
            static_cast<int>(std::round(center.x - radius)),
            static_cast<int>(std::round(center.y - radius)),
            static_cast<int>(std::round(2.0f * radius)),
            static_cast<int>(std::round(2.0f * radius)));

        rect &= cv::Rect(0, 0, originalImage.cols, originalImage.rows);
        if (rect.width <= 10 || rect.height <= 10)
            continue;

        double aspectRatio = static_cast<double>(rect.width) / std::max(1, rect.height);
        if (aspectRatio < 0.85 || aspectRatio > 1.15)
            continue;

        double enclosingArea = CV_PI * radius * radius;
        if (enclosingArea <= 0.0)
            continue;

        double areaRatio = area / enclosingArea;
        if (areaRatio < 0.52 || areaRatio > 1.20)
            continue;

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
            continue;

        cv::Scalar avgBgr = getContourColor(contours[i], originalImage);
        found.push_back({"circle", rect.x, rect.y, rect.width, rect.height, avgBgr, 0.0f});
        acceptedRects.push_back(rect);
    }

    return found;
}