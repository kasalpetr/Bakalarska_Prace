#include "ShapeDetector.hpp"

class SquareDetector : public ShapeDetector { // specific implementation for square detection
public:
    std::vector<Shape> detect(const cv::Mat& processedImage) override {
        std::vector<Shape> found;
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(processedImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for (const auto& contour : contours) {
            std::vector<cv::Point> approx;
            double peri = cv::arcLength(contour, true);
            cv::approxPolyDP(contour, approx, 0.05 * peri, true);

            if (approx.size() == 4) {
                cv::Rect rect = cv::boundingRect(approx);
                if (rect.width > 30 && rect.height > 30) {
                    found.push_back({"rectangle", rect.x, rect.y, rect.width, rect.height});
                }
            }
        }
        return found;
    }
};