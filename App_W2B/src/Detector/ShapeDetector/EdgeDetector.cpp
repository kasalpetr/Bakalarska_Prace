#include "EdgeDetector.hpp"

#include <algorithm>
#include <cmath>

std::vector<Edge> EdgeDetector::detectEdges(cv::Mat &image, const std::vector<Shape> &shapes) // function detects edges 
{
    cv::Mat grayImage;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);

    cv::Mat lineCandidates = preprocessImage(image);

    cv::Mat maskedRegions;
    cv::threshold(grayImage, maskedRegions, 250, 255, cv::THRESH_BINARY);

    cv::Mat expandKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11));
    cv::dilate(maskedRegions, maskedRegions, expandKernel);

    cv::Mat edgeLabels;
    cv::connectedComponents(maskedRegions, edgeLabels, 8, CV_32S);

    std::vector<cv::Vec4i> rawLines;
    cv::HoughLinesP(lineCandidates, rawLines, 1, CV_PI / 180, 25, 45, 20);

    std::vector<Edge> acceptedEdges;
    for (const auto &line : rawLines)
    {
        cv::Point start(line[0], line[1]);
        cv::Point end(line[2], line[3]);

        int startLabel = getNearbyMaskLabel(edgeLabels, start, 12);
        int endLabel = getNearbyMaskLabel(edgeLabels, end, 12);

        if (startLabel <= 0 || endLabel <= 0 || startLabel == endLabel)
        {
            continue;
        }

        cv::Point2f direction(static_cast<float>(end.x - start.x), static_cast<float>(end.y - start.y));
        double length = cv::norm(direction);
        if (length < 18.0 || length > 140.0)
        {
            continue;
        }

        int sourceShapeId = findClosestShapeId(start, shapes);
        if (sourceShapeId < 0)
        {
            continue;
        }

        int targetShapeId = findClosestShapeId(end, shapes, sourceShapeId);
        if (targetShapeId < 0)
        {
            continue;
        }

        Edge edge;
        edge.sourceShapeId = sourceShapeId;
        edge.targetShapeId = targetShapeId;
        edge.start = start;
        edge.end = end;
        edge.angle = static_cast<float>(std::atan2(direction.y, direction.x) * 180.0 / CV_PI);
        edge.length = length;
        acceptedEdges.push_back(edge);
    }

    std::vector<Edge> dedupedEdges;
    for (const auto &edge : acceptedEdges)
    {
        bool isDuplicate = std::any_of(
            dedupedEdges.begin(),
            dedupedEdges.end(),
            [&](const Edge &kept) { return computeEndpointDistance(edge, kept) < 20.0; });

        if (!isDuplicate)
        {
            dedupedEdges.push_back(edge);
        }
    }

    drawDetectedEdges(image, dedupedEdges);
    return dedupedEdges;
}

cv::Mat EdgeDetector::preprocessImage(const cv::Mat &image) const // function preprocesses the image to prepare it for edge detection
{
    cv::Mat grayImage;
    cv::Mat blurred;
    cv::Mat binary;

    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(grayImage, blurred, cv::Size(5, 5), 0);
    cv::adaptiveThreshold(
        blurred,
        binary,
        255,
        cv::ADAPTIVE_THRESH_GAUSSIAN_C,
        cv::THRESH_BINARY_INV,
        13,
        2);

    cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, closeKernel);
    return binary;
}

int EdgeDetector::getNearbyMaskLabel(const cv::Mat &edgeLabels, const cv::Point &point, int radius) const // function finds the label of the nearest mask region
{
    cv::Rect roi(point.x - radius, point.y - radius, 2 * radius + 1, 2 * radius + 1);
    roi &= cv::Rect(0, 0, edgeLabels.cols, edgeLabels.rows);

    if (roi.area() <= 0)
    {
        return 0;
    }

    for (int y = roi.y; y < roi.y + roi.height; ++y)
    {
        for (int x = roi.x; x < roi.x + roi.width; ++x)
        {
            int label = edgeLabels.at<int>(y, x);
            if (label > 0)
            {
                return label;
            }
        }
    }

    return 0;
}

int EdgeDetector::findClosestShapeId(const cv::Point &point, const std::vector<Shape> &shapes, int excludedShapeId) const // function finds the closest shape ID to a given point, excluding a specific shape ID
{
    constexpr double maxDistance = 60.0;

    int closestShapeId = -1;
    double bestDistance = maxDistance;

    for (const auto &shape : shapes)
    {
        if (shape.id == excludedShapeId)
        {
            continue;
        }

        const double distance = pointToShapeDistance(point, shape);
        if (distance <= bestDistance)
        {
            bestDistance = distance;
            closestShapeId = shape.id;
        }
    }

    return closestShapeId;
}

double EdgeDetector::pointToShapeDistance(const cv::Point &point, const Shape &shape) const
{
    const int left = shape.x;
    const int top = shape.y;
    const int right = shape.x + shape.width;
    const int bottom = shape.y + shape.height;

    const int dx = std::max({left - point.x, 0, point.x - right});
    const int dy = std::max({top - point.y, 0, point.y - bottom});
    return std::hypot(dx, dy);
}

double EdgeDetector::computeEndpointDistance(const Edge &a, const Edge &b) const
{
    double direct = std::max(cv::norm(cv::Point2f(a.start - b.start)), cv::norm(cv::Point2f(a.end - b.end)));
    double reverse = std::max(cv::norm(cv::Point2f(a.start - b.end)), cv::norm(cv::Point2f(a.end - b.start)));
    return std::min(direct, reverse);
}

void EdgeDetector::drawDetectedEdges(cv::Mat &image, const std::vector<Edge> &edges) const
{
    for (const auto &edge : edges)
    {
        cv::line(image, edge.start, edge.end, cv::Scalar(0, 165, 255), 2);
        cv::circle(image, edge.start, 3, cv::Scalar(255, 100, 0), -1);
        cv::circle(image, edge.end, 3, cv::Scalar(255, 100, 0), -1);
    }
}
