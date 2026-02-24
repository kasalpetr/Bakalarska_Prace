#ifndef SHAPE_HPP 
#define SHAPE_HPP
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Shape { //basic structure to hold detected shape information
    std::string type;
    int x, y, width, height;

    json to_json_object() const {
        return json{
            {"type", type},
            {"x", x},
            {"y", y},
            {"width", width},
            {"height", height}
        };
    }

    std::string toJson() const {
        return to_json_object().dump(); 
    }
};
#endif // SHAPE_HPP