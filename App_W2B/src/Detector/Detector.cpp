#include "Detector.hpp"
#include "GeometryUtils.hpp"
#include "ShapeDetector/SquareDetector.hpp"
#include "ShapeDetector/TriangleDetector.hpp"
#include "ShapeDetector/CircleDetector.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <nlohmann/json.hpp>

Detector::Detector(cv::Mat &image)
{
    shapeDetectors.push_back(std::make_unique<SquareDetector>()); // add more detectors here as needed
    shapeDetectors.push_back(std::make_unique<TriangleDetector>());
    shapeDetectors.push_back(std::make_unique<CircleDetector>());

    this->postProcessImage = preprocessImage(image);
}

std::vector<Shape> Detector::detectShapes(cv::Mat &image) // Main function to process the image and detect shapes, returning a vector of detected Shape objects
{
    std::vector<Shape> allDetected; // aggregate results from all detectors

    for (auto &algo : shapeDetectors) // detect shapes using all algorithms and aggregate results
    {
        auto shapes = algo->detect(this->postProcessImage, image); // pass both processed and original image for better detection (e.g., color info)
        for (const auto &shape : shapes)
        {
            if (!isFalsePositiveCircle(shape, allDetected))
            {
                allDetected.push_back(shape);
            }
        }
    }

    std::vector<TextRegion> textRegions = loadTextRegions("json/detectedText.json");
    allDetected = filterShapesAgainstText(allDetected, textRegions); 

    drawDetectedShapes(image, allDetected); // Draw the detected shapes on the original image for visualization

    return allDetected;
}

std::vector<Edge> Detector::detectEdges(cv::Mat &image)
{
    
    
    return std::vector<Edge>();
}

cv::Mat Detector::preprocessImage(cv::Mat &image)
{
    cv::Mat gray, blurred, binary;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    cv::adaptiveThreshold(blurred, binary, 255,
                          cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                          cv::THRESH_BINARY_INV, 13, 2);

    cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, closeKernel);
    return binary;
}

void Detector::drawDetectedShapes(cv::Mat &image, const std::vector<Shape> &shapes) const // Draw the detected shapes on the image for visualization
{
    for (const auto &shape : shapes)
    {
        if (shape.type == "rectangle" || shape.type == "sticky_note")
        {
            cv::Scalar rectColor = (shape.type == "sticky_note") ? cv::Scalar(0, 200, 255) : cv::Scalar(0, 255, 0);
            cv::rectangle(image,
                          cv::Rect(shape.x, shape.y, shape.width, shape.height),
                          rectColor,
                          3);
        }
        else if (shape.type == "triangle")
        {
            std::vector<cv::Point> points = {
                cv::Point(shape.x + shape.width / 2, shape.y),
                cv::Point(shape.x, shape.y + shape.height),
                cv::Point(shape.x + shape.width, shape.y + shape.height)};
            std::vector<std::vector<cv::Point>> contour = {points};
            cv::drawContours(image, contour, -1, cv::Scalar(0, 0, 255), 3);
        }
        else if (shape.type == "circle")
        {
            cv::Point center(shape.x + shape.width / 2, shape.y + shape.height / 2);
            int radius = std::max(1, std::min(shape.width, shape.height) / 2);
            cv::circle(image, center, radius, cv::Scalar(255, 0, 0), 3);
        }
    }
}

bool Detector::isFalsePositiveCircle(const Shape &candidate, const std::vector<Shape> &acceptedShapes) const // fix false positives circle inside rectangles
{
    if (candidate.type != "circle")
    {
        return false;
    }

    cv::Rect candidateRect(candidate.x, candidate.y, candidate.width, candidate.height);
    for (const auto &shape : acceptedShapes)
    {
        if (shape.type != "rectangle" && shape.type != "sticky_note")
        {
            continue;
        }

        cv::Rect rectangleRect(shape.x, shape.y, shape.width, shape.height);
        if (GeometryUtils::rectIntersectionOverUnion(candidateRect, rectangleRect) > 0.35)
        {
            return true;
        }
    }

    return false;
}

std::vector<Shape> Detector::filterShapesAgainstText(const std::vector<Shape> &shapes, const std::vector<TextRegion> &textRegions) const // Filter shapes against text
{
    if (textRegions.empty())
    {
        return shapes;
    }

    std::vector<Shape> filtered;
    filtered.reserve(shapes.size());

    for (const auto &shape : shapes)
    {
        if (!isShapeWronglyDetected(shape, textRegions))
        {
            filtered.push_back(shape);
        }
    }

    return filtered;
}

std::vector<Detector::TextRegion> Detector::loadTextRegions(const std::string &filePath) const //load text regions from JSON
{
    std::ifstream jsonFile(filePath);
    if (!jsonFile.is_open())
    {
        return {};
    }

    nlohmann::json jsonData;
    jsonFile >> jsonData;

    if (!jsonData.contains("responses") || !jsonData["responses"].is_array() || jsonData["responses"].empty())
    {
        return {};
    }

    const auto &response = jsonData["responses"][0];
    if (!response.contains("textAnnotations") || !response["textAnnotations"].is_array())
    {
        return {};
    }

    std::vector<TextRegion> regions;
    const auto &annotations = response["textAnnotations"];
    for (size_t i = 1; i < annotations.size(); ++i) // Skip [0], which is a global text box for all text.
    {
        const auto &annotation = annotations[i];
        if (!annotation.contains("boundingPoly"))
        {
            continue;
        }

        const auto &poly = annotation["boundingPoly"];
        if (!poly.contains("vertices") || !poly["vertices"].is_array())
        {
            continue;
        }

        std::vector<cv::Point> points;
        for (const auto &vertex : poly["vertices"])
        {
            int x = vertex.value("x", 0);
            int y = vertex.value("y", 0);
            points.emplace_back(x, y);
        }

        if (points.size() < 3)
        {
            continue;
        }

        cv::Rect box = cv::boundingRect(points);
        if (box.width <= 0 || box.height <= 0)
        {
            continue;
        }

        TextRegion region;
        region.rect = box;
        region.text = annotation.value("description", std::string());
        regions.push_back(region);
    }

    return regions;
}

bool Detector::isShapeWronglyDetected(const Shape &shape, const std::vector<TextRegion> &textRegions) const // check if the shape is wrongly detected text
{
    cv::Rect shapeRect(shape.x, shape.y, shape.width, shape.height);
    if (shapeRect.width <= 0 || shapeRect.height <= 0)
    {
        return false;
    }

    double shapeArea = static_cast<double>(shapeRect.area());

    for (const auto &region : textRegions) // check each shape againts all text
    {
        cv::Rect inter = shapeRect & region.rect;
        if (inter.area() <= 0 || region.rect.area() <= 0)
        {
            continue;
        }

        double interArea = static_cast<double>(inter.area());
        double overlapOnShape = interArea / shapeArea;
        double textArea = static_cast<double>(region.rect.area());
        double overlapOnText = interArea / textArea;

        int visibleChars = countVisibleChars(region.text);
        double shapeToTextAreaRatio = shapeArea / std::max(1.0, textArea);

        cv::Point2f shapeCenter(shapeRect.x + shapeRect.width * 0.5f,
                                shapeRect.y + shapeRect.height * 0.5f);
        cv::Point2f textCenter(region.rect.x + region.rect.width * 0.5f,
                               region.rect.y + region.rect.height * 0.5f);
        double centerDistance = cv::norm(shapeCenter - textCenter);
        double centerNorm = centerDistance /
                            std::max(1.0, static_cast<double>(std::max(shapeRect.width, shapeRect.height)));

        bool nearSameCenter = centerNorm < 0.20;
        bool similarScale = shapeToTextAreaRatio > 0.60 && shapeToTextAreaRatio < 1.60;

        bool regionLooksLikeTextLine = false;
        if (visibleChars == 1)
        {
            for (const auto &other : textRegions)
            {
                if (&other == &region)
                {
                    continue;
                }

                int otherChars = countVisibleChars(other.text);
                if (otherChars < 2)
                {
                    continue;
                }

                int yTop = std::max(region.rect.y, other.rect.y);
                int yBottom = std::min(region.rect.y + region.rect.height, other.rect.y + other.rect.height);
                double verticalOverlap = std::max(0, yBottom - yTop) /
                                         static_cast<double>(std::max(1, std::min(region.rect.height, other.rect.height)));

                int xGap = std::max({other.rect.x - (region.rect.x + region.rect.width),
                                     region.rect.x - (other.rect.x + other.rect.width),
                                     0});
                double maxGap = 1.5 * static_cast<double>(std::max(region.rect.height, other.rect.height));

                if (verticalOverlap > 0.35 && static_cast<double>(xGap) < maxGap)
                {
                    regionLooksLikeTextLine = true;
                    break;
                }
            }
        }
        // Heuristic rules to determine if the shape should be rejected based on its overlap with text regions and other factors
        bool likelyGlyphConflict = (shape.type == "circle") && (visibleChars == 1) &&
                                   overlapOnShape > 0.55 && overlapOnText > 0.55 &&
                                   nearSameCenter && similarScale && regionLooksLikeTextLine;

        bool textDominatesShape = overlapOnShape > 0.75 && shapeToTextAreaRatio < 1.15;
        bool shapeMostlyInsideText = overlapOnText > 0.85 && shapeToTextAreaRatio < 1.30;

        if (likelyGlyphConflict || textDominatesShape || shapeMostlyInsideText)
        {
            return true;
        }
    }

    return false;
}

int Detector::countVisibleChars(const std::string &text) const // Count the visible characters in the text
{
    int count = 0;
    for (char c : text)
    {
        if (!std::isspace(static_cast<unsigned char>(c)))
        {
            ++count;
        }
    }
    return count;
}
