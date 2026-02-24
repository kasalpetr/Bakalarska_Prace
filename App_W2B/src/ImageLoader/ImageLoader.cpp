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
    if (img.empty() || !isWhiteboard(img)) {
        std::cerr << "Invalid image: Image is empty or does not appear to be a whiteboard." << std::endl;
        return false;
    }
    return true;

}

bool ImageLoader::isWhiteboard(const cv::Mat &img)
{
    if (img.empty()) return false;

    cv::Mat gray, processed;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    cv::adaptiveThreshold(gray, processed, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                          cv::THRESH_BINARY_INV, 11, 2);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(processed, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double maxArea = 0;
    bool foundRect = false;
    double imgArea = img.cols * img.rows;

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        
        if (area > imgArea * 0.02) { // Minimum area threshold 2% of image area
            std::vector<cv::Point> approx;
            double peri = cv::arcLength(contour, true);
            cv::approxPolyDP(contour, approx, 0.02 * peri, true);

            if (approx.size() >= 4 && approx.size() <= 6) { 
                maxArea = area;
                foundRect = true;
                break; 
            }
        }
    }

    if (!foundRect) return false;

    int foregroundPixels = cv::countNonZero(processed);
    double contentRatio = (double)foregroundPixels / imgArea;

    if (contentRatio < 0.01 || contentRatio > 0.40) {
        return false;
    }

    cv::Scalar meanColor = cv::mean(img);
    double brightness = (meanColor[0] + meanColor[1] + meanColor[2]) / 3.0;
    
    return brightness > 130; 
}
