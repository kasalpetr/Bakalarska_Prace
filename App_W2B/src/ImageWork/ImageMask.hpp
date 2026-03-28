#include <opencv2/opencv.hpp>


class ImageMask
{
public:
    static cv::Mat createMask(const cv::Mat& img);
private:
    static cv::Mat maskShape(const cv::Mat& img, const std::string& filePath);
    static cv::Mat maskText(const cv::Mat& img, const std::string& filePath);
};