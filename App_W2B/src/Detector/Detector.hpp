#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <string>
#include "Shape.hpp"
#include "ShapeDetector/ShapeDetector.hpp"

class Detector {
private:
    struct TextRegion
    {
        cv::Rect rect;
        std::string text;
    };

    std::vector<std::unique_ptr<ShapeDetector>> shapeDetectors; // holds different shape detectors
    bool isFalsePositiveCircle(const Shape &candidate, const std::vector<Shape> &acceptedShapes) const;
    std::vector<TextRegion> loadTextRegions(const std::string &filePath) const;
    bool isShapeWronglyDetected(const Shape &shape, const std::vector<TextRegion> &textRegions) const;
    std::vector<Shape> filterShapesAgainstText(const std::vector<Shape> &shapes, const std::vector<TextRegion> &textRegions) const;
    void drawDetectedShapes(cv::Mat &image, const std::vector<Shape> &shapes) const;
    int countVisibleChars(const std::string &text) const;
public:
    Detector();
    std::vector<Shape> processImage(cv::Mat& image);
};