#include "ShapeDetector.hpp"

class SquareDetector : public ShapeDetector
{
private:
    double vertexAngleDeg(const cv::Point &a, const cv::Point &b, const cv::Point &c);
    bool hasMostlyRightAngles(const std::vector<cv::Point> &quad);

protected:
    float getContourAngle(const std::vector<cv::Point> &contour) override;

public:
    std::vector<Shape> detect(const cv::Mat &processedImage, const cv::Mat &originalImage) override;
};
