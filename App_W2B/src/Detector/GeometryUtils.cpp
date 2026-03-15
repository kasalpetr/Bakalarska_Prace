#include "GeometryUtils.hpp"

double GeometryUtils::rectIntersectionOverUnion(const cv::Rect &a, const cv::Rect &b)
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
