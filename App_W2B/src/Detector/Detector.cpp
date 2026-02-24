#include "Detector.hpp"
#include "ShapeDetector/SquareDetector.cpp"

Detector::Detector()
{
    shapeDetectors.push_back(std::make_unique<SquareDetector>()); //add more detectors here as needed
}

std::vector<Shape> Detector::processImage(const cv::Mat &image)
{
    cv::Mat gray, blurred, edged; //preprocess the image for better shape detection
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    cv::Canny(blurred, edged, 50, 150);

    std::vector<Shape> allDetected; //aggregate results from all detectors

    for (auto &algo : shapeDetectors) //detect shapes using all algorithms and aggregate results
    {
        auto shapes = algo->detect(edged);
        allDetected.insert(allDetected.end(), shapes.begin(), shapes.end());
    }

    return allDetected;
}