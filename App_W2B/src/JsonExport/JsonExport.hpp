#include <vector>
#include <string>
#include "../Detector/Shape.hpp"

class JsonExporter {
public:
    static void saveShapes(const std::vector<Shape>& shapes, const std::string& filename);
};