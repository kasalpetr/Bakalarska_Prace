#include <opencv2/opencv.hpp>
#include <iostream>
#include "ImageLoader/ImageLoader.hpp"
#include <filesystem>
#include "Detector/Detector.hpp"
#include "JsonExport/JsonExport.hpp"

int main(int argc, char *argv[])
{

    // if (argc < 2) {
    //     std::cerr << "Error: No image path provided!" << std::endl;
    //     std::cout << "Usage: " << argv[0] << " <image_path>" << std::endl;
    //     return 1;
    // }

    // std::string imagePath = argv[1];

    // cv::Mat image = ImageLoader::loadImage(imagePath);

    //
    // cv::Mat image = ImageLoader::loadImage("/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/Img/Table.png"); // Load the image
    cv::Mat image = ImageLoader::loadImage("/home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/Img/Table2.png"); // Load the image for debian

    Detector detector;
    std::vector<Shape> squares = detector.processImage(image); // Process the image to detect shapes

    std::cout << "Detected " << squares.size() << " shapes." << std::endl;

    // JsonExporter::saveShapes(squares, "../json/detected_objects.json");
    JsonExporter::saveShapes(squares, "/home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/json/detected_objects.json"); // debiani
    // system("python3 ../src/APISender/Main.py");
    system("python3 /home/kasal/Bakalarka/Bakalarska_Prace/App_W2B/src/APISender/Main.py"); // debian

    cv::imshow("Loaded Image", image); // Display the loaded image
    cv::waitKey(0);
    return 0;
}