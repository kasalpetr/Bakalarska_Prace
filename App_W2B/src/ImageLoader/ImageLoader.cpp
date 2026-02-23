#include "ImageLoader.hpp"

cv::Mat ImageLoader::loadImage(const std::string &path)
{ // Load the image using OpenCV
    cv::Mat img = cv::imread(path);

    if (!validateImage(img)) {     
        std::cerr << "Failed to load image from path: " << path << std::endl;
        return cv::Mat();
    }

    return img;
}

bool ImageLoader::validateImage(const cv::Mat &img) // Check if the image is valid (not empty)
{
    if (img.empty()) {
        return false;
    }
    return true;

    //to do check if its whiteboard, if not return false
}