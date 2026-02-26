#include <opencv2/opencv.hpp>
#include <iostream>
#include "ImageLoader/ImageLoader.hpp"
#include <filesystem>
#include "Detector/Detector.hpp"
#include "JsonExport/JsonExport.hpp"

int main()
{
    cv::Mat image = ImageLoader::loadImage("/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/Img/Table.png"); // Load the image

    Detector detector;
    std::vector<Shape> squares = detector.processImage(image); // Process the image to detect shapes

    std::cout << "Detected " << squares.size() << " shapes." << std::endl;

    JsonExporter::saveShapes(squares, "../json/detected_objects.json");
    system("python3 ../src/APISender/Main.py");
    cv::imshow("Loaded Image", image); // Display the loaded image
    cv::waitKey(0);
    return 0;
}