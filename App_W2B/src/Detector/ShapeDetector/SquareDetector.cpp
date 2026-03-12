#include "ShapeDetector.hpp"

class SquareDetector : public ShapeDetector
{ // specific implementation for square detection
protected:
    float getContourAngle(const std::vector<cv::Point> &contour) override
    {
        cv::RotatedRect rotatedRect = cv::minAreaRect(contour);
        float angle = rotatedRect.angle;
        cv::Size2f rectSize = rotatedRect.size;

        if (rectSize.width < rectSize.height)
        {
            angle += 90.0f;
        }
        return angle;
    }

public:
    std::vector<Shape> detect(const cv::Mat &processedImage, const cv::Mat &originalImage) override
    {
        std::vector<Shape> found;
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::Rect rect;     // bounding rectangle for detected contour
        float angle;       // angle of the detected square
        cv::Scalar avgBgr; // average color of the detected square
        cv::findContours(processedImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        for (size_t i = 0; i < contours.size(); ++i)
        {
            std::vector<cv::Point> approx;
            double peri = cv::arcLength(contours[i], true);           // Calculate the perimeter of the contour
            cv::approxPolyDP(contours[i], approx, 0.05 * peri, true); // Approximate contour to polygon

            if (approx.size() == 4)
            {
                cv::Rect rect = cv::boundingRect(approx); // Get bounding rectangle of the approximated polygon
                if (rect.width > 10 && rect.height > 10)
                {
                    bool isInnerStroke = false;
                    int parentIdx = hierarchy[i][3];

                    if (parentIdx != -1)
                    {
                        cv::Rect parentRect = cv::boundingRect(contours[parentIdx]);
                        if (std::abs(rect.x - parentRect.x) < 10 &&
                            std::abs(rect.width - parentRect.width) < 10)
                        {
                            isInnerStroke = true;
                        }
                    }

                    if (!isInnerStroke)
                    {
                        angle = getContourAngle(approx);
                        avgBgr = getContourColor(approx, originalImage);

                        found.push_back({"rectangle", rect.x, rect.y, rect.width, rect.height, avgBgr, angle});
                    }
                }
            }
        }
        return found;
    }
};