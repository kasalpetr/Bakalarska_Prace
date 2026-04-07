#include "ImageMask.hpp"
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

cv::Mat ImageMask::createMask(const cv::Mat &img) // Mask all shapes in the image and text, leaving only some images which are not detected as shapes or text
{
    cv::Mat mask = img.clone();                                        // Create a copy of the input image to be used as a mask
    std::string shapeJsonPath = "../App_W2B/json/detectedShapes.json"; // Path to the JSON file with detected shapes information
    std::string textJsonPath = "../App_W2B/json/detectedText.json";    // Path to the JSON file with detected text information
    mask = maskShape(mask, shapeJsonPath);                             // Mask detected shapes in the image using the JSON file with detected shapes information
    mask = maskText(mask, textJsonPath);                               // Mask detected text in the image using the JSON file with detected text information
    return mask;
}

cv::Mat ImageMask::createTransparentResidual(const cv::Mat &img, const cv::Mat &maskedImg)
{
    cv::Mat foregroundMask = extractForegroundMask(img);

    cv::Mat maskedRegions;
    cv::inRange(maskedImg, cv::Scalar(255, 255, 255), cv::Scalar(255, 255, 255), maskedRegions);

    cv::Mat keepRegions;
    cv::bitwise_not(maskedRegions, keepRegions);

    cv::Mat alpha;
    cv::bitwise_and(foregroundMask, keepRegions, alpha);

    cv::Mat residual(img.rows, img.cols, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    cv::Mat filteredAlpha = cv::Mat::zeros(alpha.size(), alpha.type());

    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    int componentCount = cv::connectedComponentsWithStats(alpha, labels, stats, centroids, 8, CV_32S);

    for (int label = 1; label < componentCount; ++label)
    {
        int area = stats.at<int>(label, cv::CC_STAT_AREA);
        if (area < 26)
        {
            continue;
        }

        cv::Mat componentMask = labels == label;
        cv::Scalar avgBgr = cv::mean(img, componentMask);
        cv::Scalar classifiedColor = classifyResidualColor(avgBgr);

        residual.setTo(cv::Scalar(classifiedColor[0], classifiedColor[1], classifiedColor[2], 255), componentMask);
        filteredAlpha.setTo(255, componentMask);
    }

    std::vector<cv::Mat> channels;
    cv::split(residual, channels);
    channels[3] = filteredAlpha;
    cv::merge(channels, residual);

    return residual;
}

cv::Scalar ImageMask::classifyResidualColor(const cv::Scalar &avgBgr)
{
    cv::Scalar red(0, 0, 255);
    cv::Scalar green(0, 255, 0);
    cv::Scalar blue(255, 0, 0);
    cv::Scalar black(0, 0, 0);
    cv::Scalar orange(0, 165, 255);

    if (avgBgr[0] > avgBgr[1] && avgBgr[0] > avgBgr[2])
        return blue;
    else if (avgBgr[1] > avgBgr[0] && avgBgr[1] > avgBgr[2])
        return green;
    else if (avgBgr[2] > avgBgr[0] && avgBgr[2] > avgBgr[1])
        return red;
    else if (avgBgr[0] > 100 && avgBgr[1] > 100 && avgBgr[2] < 100)
        return orange;
    return black;
}

cv::Mat ImageMask::extractForegroundMask(const cv::Mat &img)
{
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    cv::Mat darkOnly = gray.clone();
    for (int y = 0; y < darkOnly.rows; ++y)
    {
        unsigned char *ptr = darkOnly.ptr<unsigned char>(y);
        for (int x = 0; x < darkOnly.cols; ++x)
        {
            if (ptr[x] > 200) // Threshold for light pixels, adjust as needed
                ptr[x] = 255;
        }
    }

    cv::Mat blurred;
    cv::GaussianBlur(darkOnly, blurred, cv::Size(5, 5), 0);

    cv::Mat thresholded;
    cv::adaptiveThreshold(blurred, thresholded,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C,cv::THRESH_BINARY_INV,35,12);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::Mat cleaned;
    cv::morphologyEx(thresholded, cleaned,cv::MORPH_OPEN,kernel,cv::Point(-1, -1),2); 

    return cleaned;
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

cv::Mat ImageMask::maskText(const cv::Mat &img, const std::string &filePath) // Mask text
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
