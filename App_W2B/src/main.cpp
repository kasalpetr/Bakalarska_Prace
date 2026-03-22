#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include "ImageLoader/ImageLoader.hpp"
#include "Detector/Detector.hpp"
#include "JsonExport/JsonExport.hpp"

int main(int argc, char *argv[])
{

    // cv::Mat image = ImageLoader::loadImage("/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/Img/Shape.png"); // Load the image
    cv::Mat image = ImageLoader::loadImage("/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/Img/Table2.jpg"); // Load the image
    // cv::Mat image = ImageLoader::loadImage("/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/Img/BasicRectangle.png"); // Load the image
    // cv::Mat image = ImageLoader::loadImage("/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/Img/Rectangle.png"); // Load the image

    //call Google Vision API for text detection and save the result to JSON file
    std::string tmpPath = "/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/Img/TmpGoogleVision.png"; // Temporary path for the image to be processed by Google Vision API
    cv::imwrite(tmpPath, image);
    std::string PythonText = "python3 \"/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/src/APISender/GoogleVision.py\" \"" + tmpPath + "\"";
    system(PythonText.c_str());
    std::filesystem::remove(tmpPath); // Clean up the temporary file after processing
    
    
    // cv::Mat image = ImageLoader::loadImage("/home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/Img/Table2.png"); // Load the image for debian

    Detector detector;
    std::vector<Shape> squares = detector.processImage(image); // Process the image to detect shapes

    std::cout << "Detected " << squares.size() << " shapes." << std::endl;

    JsonExporter::saveShapes(squares, "/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/json/detectedObjects.json");
    // JsonExporter::saveShapes(squares, "/home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/json/detectedObjects.json"); // debiani

    system("python3 \"/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/src/APISender/Main.py\""); // Call the Python script to upload JSON data to API");
    // system("python3 /home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/src/APISender/Main.py"); // debian

    cv::imshow("Loaded Image", image); // Display the loaded image
    cv::waitKey(0);
    return 0;
}