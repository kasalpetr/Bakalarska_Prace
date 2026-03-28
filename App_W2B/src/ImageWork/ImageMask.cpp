#include "ImageMask.hpp"
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

cv::Mat ImageMask::createMask(const cv::Mat &img) // Mask all shapes in the image and text, leaving only some images which are not detected as shapes or text
{
    cv::Mat mask = img.clone();                                                                                 // Create a copy of the input image to be used as a mask
    std::string shapeJsonPath = "../App_W2B/json/detectedShapes.json"; // Path to the JSON file with detected shapes information
    std::string textJsonPath = "../App_W2B/json/detectedText.json";     // Path to the JSON file with detected text information
    mask = maskShape(mask, shapeJsonPath); // Mask detected shapes in the image using the JSON file with detected shapes information
    mask = maskText(mask, textJsonPath);   // Mask detected text in the image using the JSON file with detected text information
    return mask;
}

cv::Mat ImageMask::maskShape(const cv::Mat &img, const std::string &filePath) // Mask shapes
{
    cv::Mat masked = img.clone();

    std::ifstream jsonFileShape(filePath);
    if (!jsonFileShape.is_open())
    {
        std::cerr << "Failed to open JSON file for reading: " << std::endl;
        return masked; // Return the original image as a fallback if the JSON file cannot be opened
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
        width = std::min(width, masked.cols - x);
        height = std::min(height, masked.rows - y);


        // Fill the detected shape or text area with white color to mask it
        cv::rectangle(masked, cv::Rect(x, y, width, height), cv::Scalar(255, 255, 255), cv::FILLED);
    }

    return masked;
}

cv::Mat ImageMask::maskText(const cv::Mat &img, const std::string &filePath) //Mask text
{
    cv::Mat masked = img.clone();

    std::ifstream jsonFileText(filePath);

    if (!jsonFileText.is_open())
    {
        std::cerr << "Failed to open JSON file for reading: " << std::endl;
        return masked; // Return the original image as a fallback if the JSON file cannot be opened
    }

    nlohmann::json jsonData;
    jsonFileText >> jsonData;

    if (!jsonData.contains("responses") || !jsonData["responses"].is_array() || jsonData["responses"].empty())
    {
        return masked;
    }

    const auto &response = jsonData["responses"][0];
    if (!response.contains("textAnnotations") || !response["textAnnotations"].is_array())
    {
        return masked;
    }

    const auto &annotations = response["textAnnotations"];

    for (size_t i = 1; i < annotations.size(); ++i)
    {
        const auto &annotation = annotations[i];
        if (!annotation.contains("boundingPoly"))
        {
            continue;
        }

        const auto &poly = annotation["boundingPoly"];
        if (!poly.contains("vertices") || !poly["vertices"].is_array())
        {
            continue;
        }

        std::vector<cv::Point> points;
        for (const auto &vertex : poly["vertices"])
        {
            int x = vertex.value("x", 0);
            int y = vertex.value("y", 0);
            x = std::max(0, std::min(x, masked.cols - 1));
            y = std::max(0, std::min(y, masked.rows - 1));
            points.emplace_back(x, y);
        }

        if (points.size() < 3)
        {
            continue;
        }

        cv::fillPoly(masked, std::vector<std::vector<cv::Point>>{points}, cv::Scalar(255, 255, 255));
    }

    return masked;
}
