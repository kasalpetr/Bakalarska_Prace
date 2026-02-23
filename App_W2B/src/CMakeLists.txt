cmake_minimum_required(VERSION 3.10)
project(DigitalBoard)

set(CMAKE_CXX_STANDARD 17)

# Tady musíme nasměrovat CMake na tvé OpenCV
# Pokud jsi stáhl Windows balíček, nasměruj to do složky 'build' uvnitř opencv
set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/include/opencv-4.12.0/build")
find_package(OpenCV REQUIRED)

# Zahrnutí tvých složek s hlavičkami
include_directories(src/ImageLoader)

# Definice souborů projektu
add_executable(DigitalBoard 
    src/Main.cpp 
    src/ImageLoader/ImageLoader.cpp
)

# Propojení s OpenCV
target_link_libraries(DigitalBoard ${OpenCV_LIBRARIES})