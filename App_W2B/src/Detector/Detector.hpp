#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <string>
#include "ShapeAndEdges.hpp"
#include "ShapeDetector/ShapeDetector.hpp"

class Detector {
private:
    cv::Mat postProcessImage; // preprocessed image for shape detection
    struct TextRegion
    {
        cv::Rect rect;
        std::string text;
    };

    std::vector<std::unique_ptr<ShapeDetector>> shapeDetectors; // holds different shape detectors
    cv::Mat preprocessImage(cv::Mat &image);
    bool isFalsePositiveCircle(const Shape &candidate, const std::vector<Shape> &acceptedShapes) const;
    std::vector<TextRegion> loadTextRegions(const std::string &filePath) const;
    bool isShapeWronglyDetected(const Shape &shape, const std::vector<TextRegion> &textRegions) const;
    std::vector<Shape> filterShapesAgainstText(const std::vector<Shape> &shapes, const std::vector<TextRegion> &textRegions) const;
    void drawDetectedShapes(cv::Mat &image, const std::vector<Shape> &shapes) const;
    int countVisibleChars(const std::string &text) const;
public:
    Detector(cv::Mat& image);
    std::vector<Shape> detectShapes(cv::Mat& image);
    std::vector<Edge> detectEdges(cv::Mat& image);
};