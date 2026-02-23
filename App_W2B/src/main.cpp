#include <opencv2/opencv.hpp>
#include <iostream>
#include "ImageLoader/ImageLoader.hpp"
#include <filesystem>
#include "DetectShape/Detector.hpp"
#include "JsonExport/JsonExport.hpp"

int main() {
    ImageLoader loader;
    cv::Mat image = loader.loadImage("/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/Img/Table.jpg");

    
    cv::imshow("Loaded Image", image);
    cv::waitKey(0);

    Detector detector;
    std::vector<Shape> squares = detector.detectSquares(image);

    std::cout << "Detected " << squares.size() << " shapes." << std::endl;
    JsonExporter::saveShapes(squares, "../json/detected_objects.json");

    return 0;
}