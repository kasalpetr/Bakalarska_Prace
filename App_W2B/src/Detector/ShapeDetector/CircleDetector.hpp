#include "ShapeDetector.hpp"

class CircleDetector : public ShapeDetector
{
protected:
    float getContourAngle(const std::vector<cv::Point> &contour) override;

public:
    std::vector<Shape> detect(const cv::Mat &processedImage, const cv::Mat &originalImage) override;
};
