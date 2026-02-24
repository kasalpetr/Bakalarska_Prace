#ifndef SHAPE_HPP 
#define SHAPE_HPP

#include <string>

struct Shape // basic structure to hold detected shape information
{
    std::string type;
    int x, y, width, height;
    

    std::string toJson() const
    {
        return "{\"type\": \"" + type + "\", \"x\": " + std::to_string(x) +
               ", \"y\": " + std::to_string(y) + ", \"width\": " + std::to_string(width) +
               ", \"height\": " + std::to_string(height) + "}";
    }
};

#endif // SHAPE_HPP