cmake_minimum_required(VERSION 3.10)
project(DigitalBoard)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(src/ImageWork)


add_executable(DigitalBoard 
    src/Main.cpp 
    src/ImageWork/ImageLoader.cpp
    src/ImageWork/ImageMask.cpp
    src/Detector/Detector.cpp
    src/Detector/ShapeDetector/EdgeDetector.cpp
    src/Detector/GeometryUtils.cpp
    src/JsonExport/JsonExporter.cpp
    src/Detector/ShapeDetector/ShapeDetector.hpp
    src/Detector/ShapeDetector/SquareDetector.hpp
    src/Detector/ShapeDetector/TriangleDetector.hpp
    src/Detector/ShapeDetector/CircleDetector.hpp
    src/Detector/ShapeDetector/SquareDetector.cpp
    src/Detector/ShapeDetector/TriangleDetector.cpp
    src/Detector/ShapeDetector/CircleDetector.cpp
    src/Test/TestRunner.cpp
    )

target_compile_definitions(DigitalBoard PRIVATE APP_W2B_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}\")

target_link_libraries(DigitalBoard ${OpenCV_LIBRARIES} nlohmann_json::nlohmann_json)