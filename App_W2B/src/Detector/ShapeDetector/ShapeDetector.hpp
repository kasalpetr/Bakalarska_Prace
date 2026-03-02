#ifndef SHAPEDETECTOR_HPP
#define SHAPEDETECTOR_HPP
#include <opencv2/opencv.hpp>
#include <vector>
#include "../Shape.hpp"

class ShapeDetector
{ // virtual methods for different shape detection algorithms
public:
    virtual ~ShapeDetector() = default;
    virtual std::vector<Shape> detect(const cv::Mat &processedImage, const cv::Mat &originalImage) = 0;

protected:
    virtual float getContourAngle(const std::vector<cv::Point> &contour)
    { // default implementation to get angle of a contour
        if (contour.empty())
            return 0.0f;
        cv::RotatedRect rect = cv::minAreaRect(contour);
        return rect.angle;
    }

    virtual cv::Scalar getContourColor(const std::vector<cv::Point> &contour, const cv::Mat &originalImage)
    { // default implementation to get average color of a contour

        if (contour.empty())
            return cv::Scalar(0, 0, 0);
        cv::Mat mask = cv::Mat::zeros(originalImage.size(), CV_8UC1);
        std::vector<std::vector<cv::Point>> contours{contour};
        cv::drawContours(mask, contours, 0, cv::Scalar(255), 3);
        cv::Scalar avgBgr = cv::mean(originalImage, mask);
        double threshold = 140.0;

        if (avgBgr[0] < threshold && avgBgr[1] < threshold && avgBgr[2] < threshold) // if the color is too dark, return black
        {
            return cv::Scalar(0, 0, 0);
        }
        return avgBgr;
    }
};

#endif // SHAPEDETECTOR_HPP