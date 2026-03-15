cmake_minimum_required(VERSION 3.10)
project(DigitalBoard)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(src/ImageLoader)

add_executable(DigitalBoard 
    src/main.cpp 
    src/ImageLoader/ImageLoader.cpp
    src/Detector/Detector.cpp
    src/Detector/GeometryUtils.cpp
    src/JsonExport/JsonExport.cpp
    src/Detector/ShapeDetector/ShapeDetector.hpp
    src/Detector/ShapeDetector/SquareDetector.hpp
    src/Detector/ShapeDetector/TriangleDetector.hpp
    src/Detector/ShapeDetector/CircleDetector.hpp
    src/Detector/ShapeDetector/SquareDetector.cpp
    src/Detector/ShapeDetector/TriangleDetector.cpp
    src/Detector/ShapeDetector/CircleDetector.cpp
    )

target_link_libraries(DigitalBoard ${OpenCV_LIBRARIES} nlohmann_json::nlohmann_json)