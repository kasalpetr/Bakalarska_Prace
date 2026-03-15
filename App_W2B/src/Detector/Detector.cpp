#include "Detector.hpp"
#include "GeometryUtils.hpp"
#include "ShapeDetector/SquareDetector.hpp"
#include "ShapeDetector/TriangleDetector.hpp"
#include "ShapeDetector/CircleDetector.hpp"

#include <algorithm>

bool Detector::isFalsePositiveCircle(const Shape &candidate, const std::vector<Shape> &acceptedShapes) const
{
    if (candidate.type != "circle")
    {
        return false;
    }

    cv::Rect candidateRect(candidate.x, candidate.y, candidate.width, candidate.height);
    for (const auto &shape : acceptedShapes)
    {
        if (shape.type != "rectangle")
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

Detector::Detector()
{
    shapeDetectors.push_back(std::make_unique<SquareDetector>()); // add more detectors here as needed
    shapeDetectors.push_back(std::make_unique<TriangleDetector>());
    shapeDetectors.push_back(std::make_unique<CircleDetector>());
}

void Detector::drawDetectedShapes(cv::Mat &image, const std::vector<Shape> &shapes) const // Draw the detected shapes on the image for visualization
{
    for (const auto &shape : shapes)
    {
        if (shape.type == "rectangle")
        {
            cv::rectangle(image,
                          cv::Rect(shape.x, shape.y, shape.width, shape.height),
                          cv::Scalar(0, 255, 0),
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

std::vector<Shape> Detector::processImage(cv::Mat &image)
{
    cv::Mat gray, blurred, binary; // preprocess the image for better shape detection
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    cv::adaptiveThreshold(blurred, binary, 255,
                          cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                          cv::THRESH_BINARY_INV, 13, 2);

    cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, closeKernel);

    std::vector<Shape> allDetected; // aggregate results from all detectors

    for (auto &algo : shapeDetectors) // detect shapes using all algorithms and aggregate results
    {
        auto shapes = algo->detect(binary, image); // pass both processed and original image for better detection (e.g., color info)
        for (const auto &shape : shapes)
        {
            if (!isFalsePositiveCircle(shape, allDetected))
            {
                allDetected.push_back(shape);
            }
        }
    }

    drawDetectedShapes(image, allDetected); // Draw the detected shapes on the original image for visualization

    return allDetected;
}