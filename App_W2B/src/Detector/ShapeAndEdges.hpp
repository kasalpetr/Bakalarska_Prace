#ifndef SHAPEAND_EDGES_HPP
#define SHAPEAND_EDGES_HPP 
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

using json = nlohmann::json;

struct Shape { //basic structure to hold detected shape information
    int id = -1;
    std::string type;
    int x, y, width, height;
    cv::Scalar color; // BGR color format
    float angle;

    json to_json_object() const { // Convert the Shape object to a JSON object
        return json{
            {"id", id},
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

struct Edge { //basic structure to hold detected edge information
    int sourceShapeId = -1;
    int targetShapeId = -1;
    cv::Point start;
    cv::Point end;
    float angle;
    double length;

    json to_json_object() const { // Convert the Edge object to a JSON object
        return json{
            {"sourceShapeId", sourceShapeId},
            {"targetShapeId", targetShapeId},
            {"start", {{"x", start.x}, {"y", start.y}}},
            {"end", {{"x", end.x}, {"y", end.y}}},
            {"angle", angle},
            {"length", length}
        };
    }

    std::string toJson() const {
        return to_json_object().dump(); 
    }
};
#endif // SHAPEAND_EDGES_HPP