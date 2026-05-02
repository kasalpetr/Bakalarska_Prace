#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

#include "../ShapeAndEdges.hpp"

class EdgeDetector
{
public:
   std::vector<Edge> detectEdges(cv::Mat &image, const std::vector<Shape> &shapes);

private:
    cv::Mat preprocessImage(const cv::Mat &image) const;
    int getNearbyMaskLabel(const cv::Mat &edgeLabels, const cv::Point &point, int radius) const;
    int findClosestShapeId(const cv::Point &point, const std::vector<Shape> &shapes, int excludedShapeId = -1) const;
    double pointToShapeDistance(const cv::Point &point, const Shape &shape) const;
    double computeEndpointDistance(const Edge &a, const Edge &b) const;
    double shapeBoundingBoxDistance(const Shape &a, const Shape &b) const;
    bool edgeExistsBetweenShapes(const std::vector<Edge> &edges, int shapeId1, int shapeId2) const;
    void drawDetectedEdges(cv::Mat &image, const std::vector<Edge> &edges) const;
};
