#include <opencv2/opencv.hpp>
#include <string>

class ImageLoader {
public:
    static cv::Mat loadImage(const std::string& path);
    
private:
    static bool validateImage(const cv::Mat& img);
    static bool isWhiteboard(const cv::Mat& img);
};