#include "Detector.hpp"

std::vector<Shape> Detector::detectSquares(const cv::Mat& image) {
    std::vector<Shape> foundShapes;
    cv::Mat gray, blurred, edged;

    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    cv::Canny(blurred, edged, 50, 150);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edged, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        std::vector<cv::Point> approx;
        double peri = cv::arcLength(contour, true);
        cv::approxPolyDP(contour, approx, 0.02 * peri, true);

        if (approx.size() == 4) { 
            cv::Rect rect = cv::boundingRect(approx);
            if (rect.width > 30 && rect.height > 30) {
                foundShapes.push_back({"rectangle", rect.x, rect.y, rect.width, rect.height});
            }
        }
    }
    return foundShapes;
}