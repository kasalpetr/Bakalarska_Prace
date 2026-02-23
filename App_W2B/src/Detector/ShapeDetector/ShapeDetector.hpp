#ifndef SHAPEDETECTOR_HPP
#define SHAPEDETECTOR_HPP
#include <opencv2/opencv.hpp>
#include <vector>
#include "../Shape.hpp"

class ShapeDetector { // virtual methods for different shape detection algorithms
public:
    virtual ~ShapeDetector() = default;
    virtual std::vector<Shape> detect(const cv::Mat& processedImage) = 0;
};

#endif // SHAPEDETECTOR_HPP