#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include "ImageWork/ImageLoader.hpp"
#include "Detector/Detector.hpp"
#include "JsonExport/JsonExporter.hpp"
#include "ImageWork/ImageMask.hpp"

int main(int argc, char *argv[])
{
    const std::filesystem::path appRoot(APP_W2B_ROOT);

    try
    {
        std::filesystem::current_path(appRoot);
    }
    catch (const std::filesystem::filesystem_error &error)
    {
        std::cerr << "Failed to set working directory to " << appRoot << ": " << error.what() << std::endl;
        return 1;
    }

    // cv::Mat image = ImageLoader::loadImage((appRoot / "Img" / "Shape.png").string()); // Load the image
    cv::Mat image = ImageLoader::loadImage((appRoot / "Img" / "TextShape2.jpg").string()); // Load the image
    // cv::Mat image = ImageLoader::loadImage((appRoot / "Img" / "BasicRectangle.png").string()); // Load the image
    // cv::Mat image = ImageLoader::loadImage((appRoot / "Img" / "Rectangle.png").string()); // Load the image

    //call Google Vision API for text detection and save the result to JSON file
    std::string tmpPath = (appRoot / "Img" / "TmpGoogleVision.png").string(); // Temporary path for the image to be processed by Google Vision API
    cv::imwrite(tmpPath, image);
    std::string PythonText = "python3 \"" + (appRoot / "src" / "APISender" / "GoogleVision.py").string() + "\" \"" + tmpPath + "\"";
    system(PythonText.c_str());
    std::filesystem::remove(tmpPath); // Clean up the temporary file after processing
    
    
    // cv::Mat image = ImageLoader::loadImage("/home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/Img/Table2.png"); // Load the image for debian

    Detector detector;
    std::vector<Shape> squares = detector.processImage(image); // Process the image to detect shapes

    std::cout << "Detected " << squares.size() << " shapes." << std::endl;

    JsonExporter::saveShapes(squares, (appRoot / "json" / "detectedShapes.json").string());
    // JsonExporter::saveShapes(squares, "/home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/json/detectedShapes.json"); // debiani

    system((std::string("python3 \"") + (appRoot / "src" / "APISender" / "Main.py").string() + "\"").c_str()); // Call the Python script to upload JSON data to API");
    // system("python3 /home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/src/APISender/Main.py"); // debian

    cv::imshow("Loaded Image", image); // Display the loaded image
    cv::Mat mask = ImageMask::createMask(image); // Create a mask from the image
    cv::imshow("Mask", mask); // Display the created mask
    
    cv::waitKey(0);
    return 0;
}