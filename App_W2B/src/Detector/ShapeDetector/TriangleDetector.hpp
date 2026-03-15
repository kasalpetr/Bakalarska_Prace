#include "ShapeDetector.hpp"

class TriangleDetector : public ShapeDetector
{
protected:
    float getContourAngle(const std::vector<cv::Point> &contour) override;
    double vertexAngleDeg(const cv::Point &a, const cv::Point &b, const cv::Point &c);

public:
    std::vector<Shape> detect(const cv::Mat &processedImage, const cv::Mat &originalImage) override;
};
