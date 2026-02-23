#include <opencv2/opencv.hpp>
#include <vector>
#include "Shape.hpp"
#include "ShapeDetector/ShapeDetector.hpp"

class Detector {
private:
std::vector<std::unique_ptr<ShapeDetector>> shapeDetectors; // holds different shape detectors

public:
    Detector();
    std::vector<Shape> processImage(const cv::Mat& image);
};