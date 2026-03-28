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

        if (contour.empty()) // If the contour is empty, return black as the default color
            return cv::Scalar(0, 0, 0);

        cv::Mat mask = cv::Mat::zeros(originalImage.size(), CV_8UC1);
        std::vector<std::vector<cv::Point>> contours{contour};
        cv::drawContours(mask, contours, 0, cv::Scalar(255), 3);
        cv::Scalar avgBgr = cv::mean(originalImage, mask);

        return classify(avgBgr);
    }

    cv::Scalar classify(const cv::Scalar &avgBgr) // Classify the average BGR color into a predefined set of colors 
    {
        cv::Scalar red(0, 0, 255);
        cv::Scalar green(0, 255, 0);
        cv::Scalar blue(255, 0, 0);
        cv::Scalar black(0, 0, 0);
        cv::Scalar orange(0, 165, 255);
        // OpenCV order: B,G,R
        if (avgBgr[0] > avgBgr[1] && avgBgr[0] > avgBgr[2])
            return blue;
        else if (avgBgr[1] > avgBgr[0] && avgBgr[1] > avgBgr[2])
            return green;
        else if (avgBgr[2] > avgBgr[0] && avgBgr[2] > avgBgr[1])
            return red;
        else if (avgBgr[0] > 100 && avgBgr[1] > 100 && avgBgr[2] < 100)
            return orange;        
        return black;
    }
};

#endif // SHAPEDETECTOR_HPP