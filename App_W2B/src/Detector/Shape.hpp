#ifndef SHAPE_HPP 
#define SHAPE_HPP
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

using json = nlohmann::json;

struct Shape { //basic structure to hold detected shape information
    std::string type;
    int x, y, width, height;
    cv::Scalar color; // BGR color format
    float angle;

    json to_json_object() const { // Convert the Shape object to a JSON object
        return json{
            {"type", type},
            {"x", x},
            {"y", y},
            {"width", width},
            {"height", height},
            // {"angle", angle},
            {"colorHex", colorToHex()} // Add color in hex format for better readability
        };
    }

    std::string toJson() const {
        return to_json_object().dump(); 
    }

    std::string colorToHex() const {
        char hexColor[8];
        snprintf(hexColor, sizeof(hexColor), "#%02X%02X%02X", (int)color[2], (int)color[1], (int)color[0]);
        return std::string(hexColor);
    }


};
#endif // SHAPE_HPP