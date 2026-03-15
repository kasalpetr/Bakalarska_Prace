#include <opencv2/opencv.hpp>

class GeometryUtils
{
public:
    static double rectIntersectionOverUnion(const cv::Rect &a, const cv::Rect &b);
};