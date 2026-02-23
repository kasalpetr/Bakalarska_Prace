#include <opencv2/opencv.hpp>
#include <vector>
#include "Shape.hpp"

class Detector {
public:
    std::vector<Shape> detectSquares(const cv::Mat& image);
};