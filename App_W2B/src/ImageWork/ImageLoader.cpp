#include "ImageLoader.hpp"
#include <fstream>

void ImageLoader::saveImageConfig(const cv::Mat &img, const std::string &imagePath, const std::string &outputPath)
{
    if (outputPath.empty())
    {
        return;
    }

    std::ofstream out(outputPath);
    if (!out.is_open())
    {
        std::cerr << "Failed to write image config to " << outputPath << std::endl;
        return;
    }

    out << "{\"width\": " << img.cols
        << ", \"height\": " << img.rows
        << ", \"path\": \"" << imagePath << "\"}";
}

cv::Mat ImageLoader::loadImage(const std::string &path, const std::string &imageSizeJsonPath)
{ // Load the image using OpenCV
    cv::Mat img = cv::imread(path);

    if (img.empty())
    {
        std::cerr << "Failed to load image from path: " << path << std::endl;
        return cv::Mat();
    }

    const int max_width = 1600;
    const int max_height = 1000;

    double scale = std::min((double)max_width / img.cols, (double)max_height / img.rows);

    if (scale < 1.0)
    {
        cv::Mat resizedImg;
        int new_width = static_cast<int>(img.cols * scale);
        int new_height = static_cast<int>(img.rows * scale);

        cv::resize(img, resizedImg, cv::Size(new_width, new_height), 0, 0, cv::INTER_AREA);

        std::cout << "Image resized from " << img.cols << "x" << img.rows
                  << " to " << new_width << "x" << new_height << std::endl;

        if (!validateImage(resizedImg))
        {
            std::cerr << "Failed to load image from path: " << path << std::endl;
            return cv::Mat();
        }

        saveImageConfig(resizedImg, path, imageSizeJsonPath);
        return resizedImg;
    }

    if (!validateImage(img))
    {
        std::cerr << "Failed to load image from path: " << path << std::endl;
        return cv::Mat();
    }

    saveImageConfig(img, path, imageSizeJsonPath);
    return img;
}

bool ImageLoader::validateImage(const cv::Mat &img) // Check if the image is valid (not empty)
{
    if (img.empty() || !isWhiteboard(img))
    {
        std::cerr << "Invalid image: Image is empty or does not appear to be a whiteboard." << std::endl;
        return false;
    }
    return true;
}

bool ImageLoader::isWhiteboard(const cv::Mat &image) // Heuristic to determine if the image is likely a whiteboard
{
    if (image.empty())
        return false;

    cv::Mat grayImage, blurred, binary;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(grayImage, blurred, cv::Size(5, 5), 0);
    cv::adaptiveThreshold(blurred, binary, 255,
                          cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                          cv::THRESH_BINARY_INV, 13, 2);

    cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, closeKernel);
    double imgArea = image.cols * image.rows;

    int foregroundPixels = cv::countNonZero(binary);
    double contentRatio = (double)foregroundPixels / imgArea;

    if (contentRatio < 0.002 || contentRatio > 0.45)
    {
        return false;
    }

    cv::Mat boardMask;
    cv::bitwise_not(binary, boardMask);

    double boardCoverage = static_cast<double>(cv::countNonZero(boardMask)) / imgArea;
    if (boardCoverage < 0.55)
    {
        return false;
    }

    cv::Scalar meanColor = cv::mean(image, boardMask);
    double brightness = (meanColor[0] + meanColor[1] + meanColor[2]) / 3.0;

    return brightness > 140;
}
