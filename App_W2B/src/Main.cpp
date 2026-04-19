#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include "ImageWork/ImageLoader.hpp"
#include "Detector/Detector.hpp"
#include "JsonExport/JsonExporter.hpp"
#include "ImageWork/ImageMask.hpp"

#ifndef APP_W2B_ROOT
#define APP_W2B_ROOT "/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B"
#endif

int main(int argc, char *argv[])
{
    const std::filesystem::path appRoot(APP_W2B_ROOT);

    // Ensure the working directory is set to the application root for consistent file access
    try
    {
        std::filesystem::current_path(appRoot);
    }
    catch (const std::filesystem::filesystem_error &error)
    {
        std::cerr << "Failed to set working directory to " << appRoot << ": " << error.what() << std::endl;
        return 1;
    }

    std::string imagePath; // Path to the input image, will be set by UI or hardcoded for testing

    // Launch Python UI for image selection — saves chosen path into imageConfig.json
    std::string uiCall = "python3 \"" + (appRoot / "src" / "UI" / "UI.py").string() + "\"";
    if (system(uiCall.c_str()) != 0)
    {
        std::cerr << "Image selection cancelled or UI failed." << std::endl;
        return 1;
    }
    std::ifstream configFile((appRoot / "json" / "imageConfig.json").string());
    nlohmann::json config;
    configFile >> config;
    imagePath = config["path"].get<std::string>();

   
    // imagePath = (appRoot / "Img" / "table.jpg").string(); // For testing without UI, 

    cv::Mat image = ImageLoader::loadImage(imagePath, (appRoot / "json" / "imageConfig.json").string()); // Load the image selected via UI
    Detector detector(image);                                                                            // Create a Detector object with the loaded image
    std::vector<Shape> shapes;
    std::vector<Edge> edges;

    // call Google Vision API for text detection and save the result to JSON file
    std::string tmpPath = (appRoot / "Img" / "TmpGoogleVision.png").string(); // Temporary path for the image to be processed by Google Vision API
    cv::imwrite(tmpPath, image);
    std::string PythonText = "python3 \"" + (appRoot / "src" / "APISender" / "GoogleVision.py").string() + "\" \"" + tmpPath + "\"";
    system(PythonText.c_str());
    std::filesystem::remove(tmpPath); // Clean up the temporary file after processing

    //detector
    shapes = detector.detectShapes(image); // Detect shapes in the image and get their information as a vector of Shape objects
    JsonExporter::saveShapes(shapes, (appRoot / "json" / "detectedShapes.json").string());
    std::cout << "Detected " << shapes.size() << " shapes." << std::endl;

    //mask
    cv::Mat mask = ImageMask::createMask(image); // Create a mask from the image for edge detection
    cv::Mat residual = ImageMask::createTransparentResidual(image, mask);
    cv::imwrite((appRoot / "Img" / "residual.png").string(), residual);

    //edges
    edges = detector.detectEdges(mask, shapes); // Detect edges and bind them directly to detected shapes by internal IDs
    JsonExporter::saveEdges(edges, (appRoot / "json" / "detectedEdges.json").string());
    std::cout << "Detected " << edges.size() << " edges." << std::endl;

    system((std::string("python3 \"") + (appRoot / "src" / "APISender" / "Main.py").string() + "\"").c_str()); // Call the Python script to upload JSON data to API");
    // cv::imshow("Loaded Image", image); // Display the loaded image
    cv::imshow("Mask", mask); // Display the created mask

    cv::waitKey(0);
    return 0;
}