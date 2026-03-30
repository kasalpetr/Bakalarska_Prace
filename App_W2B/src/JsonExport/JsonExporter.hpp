#include <vector>
#include <string>
#include "../Detector/ShapeAndEdges.hpp"

class JsonExporter {
public:
    static void saveShapes(const std::vector<Shape>& shapes, const std::string& filename);
    static void saveEdges(const std::vector<Edge>& edges, const std::string& filename);
};