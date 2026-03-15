#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include "Shape.hpp"
#include "ShapeDetector/ShapeDetector.hpp"

class Detector {
private:
    std::vector<std::unique_ptr<ShapeDetector>> shapeDetectors; // holds different shape detectors
    bool isFalsePositiveCircle(const Shape &candidate, const std::vector<Shape> &acceptedShapes) const;
    void drawDetectedShapes(cv::Mat &image, const std::vector<Shape> &shapes) const;

public:
    Detector();
    std::vector<Shape> processImage(cv::Mat& image);
};