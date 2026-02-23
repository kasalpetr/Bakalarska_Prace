#include <opencv2/opencv.hpp>
#include <iostream>
#include "ImageLoader/ImageLoader.hpp"
#include <filesystem>

int main() {
    ImageLoader loader;
    std::cout << "Current path is: " << std::filesystem::current_path() << std::endl;
    cv::Mat image = loader.loadImage("/mnt/c/FIT CVUT/bakalarka/Bakalarska_Prace/App_W2B/Img/Table.jpg");

    cv::imshow("Loaded Image", image);
    cv::waitKey(0);
    return 0;
}