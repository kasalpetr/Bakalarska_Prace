#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <nlohmann/json.hpp>
#include "ImageWork/ImageLoader.hpp"
#include "Detector/Detector.hpp"
#include "JsonExport/JsonExporter.hpp"
#include "ImageWork/ImageMask.hpp"

#ifndef APP_W2B_ROOT
#define APP_W2B_ROOT "./" // Default to current directory if APP_W2B_ROOT is not defined
#endif

int main(int argc, char *argv[])
{
    const std::filesystem::path appRoot(APP_W2B_ROOT);
    const std::vector<std::string> batchImagePaths = {
        // // Add test images here when you want to run the pipeline over multiple photos.k
         (appRoot / "Img" / "Rectangle.jpg").string(),
        (appRoot / "Img" / "3Shapes.jpg").string(),
        (appRoot / "Img" / "Circle.jpg").string(),
        (appRoot / "Img" / "Edge1.jpg").string(),
        (appRoot / "Img" / "Edge2.jpg").string(),
        (appRoot / "Img" / "GraphText.jpg").string(),
        (appRoot / "Img" / "RectangleText.jpg").string(),
        (appRoot / "Img" / "Triangle.jpg").string(),
        // (appRoot / "Img" / "StickyNote.jpg").string(),
        (appRoot / "Img" / "KomplexTable.jpg").string(),
    };

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

    std::vector<std::string> imagePaths;

    if (!batchImagePaths.empty())
    {
        imagePaths = batchImagePaths;
    }
    else
    {
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
        imagePaths.push_back(config["path"].get<std::string>());
    }

    for (size_t index = 0; index < imagePaths.size(); ++index)
    {
        const std::string &imagePath = imagePaths[index];
        std::cout << "Processing image " << (index + 1) << "/" << imagePaths.size() << ": " << imagePath << std::endl;

        cv::Mat image = ImageLoader::loadImage(imagePath, (appRoot / "json" / "imageConfig.json").string()); // Load the image selected via UI
        if (image.empty())
        {
            std::cerr << "Skipping image because it could not be loaded: " << imagePath << std::endl;
            continue;
        }

        Detector detector(image); // Create a Detector object with the loaded image
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
        
        //edges
        edges = detector.detectEdges(mask, shapes); // Detect edges and bind them directly to detected shapes by internal IDs
        JsonExporter::saveEdges(edges, (appRoot / "json" / "detectedEdges.json").string());
        std::cout << "Detected " << edges.size() << " edges." << std::endl;
        
        mask = ImageMask::maskEdges(mask, edges); // Mask detected edges on the mask to remove them from the residual calculation
        cv::Mat residual = ImageMask::createTransparentResidual(image, mask); // Create a transparent residual image that highlights the areas not covered by detection
        cv::imwrite((appRoot / "Img" / "residual.png").string(), residual);

        system((std::string("python3 \"") + (appRoot / "src" / "APISender" / "Main.py").string() + "\"").c_str()); // Call the Python script to upload JSON data to API");

        if (batchImagePaths.empty())
        {
            cv::imshow("Mask", mask); // Display the created mask
            cv::waitKey(0);
        }
    }

    return 0;
}