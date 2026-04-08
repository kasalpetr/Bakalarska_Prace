#include <string>
#include <opencv2/opencv.hpp>

class ImageLoader {
public:
    static cv::Mat loadImage(const std::string& path, const std::string& imageSizeJsonPath = "");
    
private:
    static void saveImageConfig(const cv::Mat& img, const std::string& imagePath, const std::string& outputPath);
    static bool validateImage(const cv::Mat& img);
    static bool isWhiteboard(const cv::Mat& img);
};