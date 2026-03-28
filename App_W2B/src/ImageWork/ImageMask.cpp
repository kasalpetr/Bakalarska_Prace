#include "ImageMask.hpp"
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>

cv::Mat ImageMask::createMask(const cv::Mat &img) // Mask all shapes in the image and text, leaving only some images which are not detected as shapes or text
{
    cv::Mat mask = img.clone();                                                                                 // Create a copy of the input image to be used as a mask
    std::string shapeJsonPath = "/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/json/detectedObjects.json"; // Path to the JSON file with detected shapes information
    std::string textJsonPath = "/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/json/detectedText.json";     // Path to the JSON file with detected text information
    // std::string shapeJsonPath = "/home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/json/detectedObjects.json"; // debian
    // std::string textJsonPath = "/home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/json/detectedText.json"; // debian
    mask = maskShape(mask, shapeJsonPath); // Mask detected shapes in the image using the JSON file with detected shapes information
    mask = maskText(mask, textJsonPath);   // Mask detected text in the image using the JSON file with detected text information
    return mask;
}

cv::Mat ImageMask::maskShape(const cv::Mat &img, const std::string &filePath) // Mask shapes
{
    std::ifstream jsonFileShape(filePath);
    // std::ifstream jsonFile("/home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/json/detectedObjects.json"); // debian
    if (!jsonFileShape.is_open())
    {
        std::cerr << "Failed to open JSON file for reading: " << std::endl;
        return img; // Return the original image as a fallback if the JSON file cannot be opened
    }

    nlohmann::json jsonData;
    jsonFileShape >> jsonData;
    for (const auto &item : jsonData)
    {
        int x = item["x"];
        int y = item["y"];
        int width = item["width"];
        int height = item["height"];

        // Ensure the rectangle is within the bounds of the image
        x = std::max(0, x);
        y = std::max(0, y);
        width = std::min(width, img.cols - x);
        height = std::min(height, img.rows - y);

        // Fill the detected shape or text area with white color to mask it
        cv::rectangle(img, cv::Rect(x, y, width, height), cv::Scalar(255, 255, 255), cv::FILLED);
    }

    return img;
}

cv::Mat ImageMask::maskText(const cv::Mat &img, const std::string &filePath) //Mask text
{
    std::ifstream jsonFileText(filePath);

    if (!jsonFileText.is_open())
    {
        std::cerr << "Failed to open JSON file for reading: " << std::endl;
        return img; // Return the original image as a fallback if the JSON file cannot be opened
    }
    nlohmann::json jsonData;
    jsonFileText >> jsonData;
    
    


    return img;
}
