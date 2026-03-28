#include "GeometryUtils.hpp"
#include <algorithm>
#include <cmath>

double GeometryUtils::rectIntersectionOverUnion(const cv::Rect &a, const cv::Rect &b) // Calculate the Intersection over Union (IoU) of two shape
{
    cv::Rect inter = a & b;
    if (inter.area() <= 0) 
    {
        return 0.0;
    }

    double unionArea = static_cast<double>(a.area() + b.area() - inter.area());
    if (unionArea <= 0.0)
    {
        return 0.0;
    }

    return static_cast<double>(inter.area()) / unionArea;
}

bool GeometryUtils::areLikelyDoubleEdgeRectangles(const cv::Rect &a, const cv::Rect &b) // Double edge rectangles
{
    double areaA = static_cast<double>(a.area()); 
    double areaB = static_cast<double>(b.area());
    if (areaA <= 0.0 || areaB <= 0.0)
    {
        return false;
    }

    cv::Point2f centerA(a.x + a.width * 0.5, a.y + a.height * 0.5); 
    cv::Point2f centerB(b.x + b.width * 0.5, b.y + b.height * 0.5);
    double centerDistance = cv::norm(centerA - centerB); 

    double maxDim = static_cast<double>(std::max(std::max(a.width, a.height),
                                                 std::max(b.width, b.height))); // 
    if (maxDim <= 0.0)
    {
        return false;
    }

    double areaRatio = std::min(areaA, areaB) / std::max(areaA, areaB);
    bool nearConcentric = centerDistance < 0.10 * maxDim;
    bool similarScale = areaRatio > 0.55;

    bool oneContainsOther = (a.contains(b.tl()) && a.contains(b.br())) ||
                            (b.contains(a.tl()) && b.contains(a.br()));

    return nearConcentric && similarScale && oneContainsOther;
}
