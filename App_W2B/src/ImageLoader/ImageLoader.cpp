#include "ImageLoader.hpp"

cv::Mat ImageLoader::loadImage(const std::string &path)
{
    cv::Mat img = cv::imread(path);

    if (!validateImage(img)) {     
        std::cerr << "Failed to load image from path: " << path << std::endl;
        return cv::Mat();
    }

    return img;
}

bool ImageLoader::validateImage(const cv::Mat &img)
{
    if (img.empty()) {
        return false;
    }
    return true;
}