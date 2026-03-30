#include "JsonExporter.hpp"
#include <fstream>

void JsonExporter::saveShapes(const std::vector<Shape>& shapes, const std::string& filename) {
    std::ofstream file(filename);
    file << "[\n";
    for (size_t i = 0; i < shapes.size(); ++i) {
        file << "  " << shapes[i].toJson() << (i < shapes.size() - 1 ? "," : "") << "\n";
    }
    file << "]";
    file.close();
}

void JsonExporter::saveEdges(const std::vector<Edge>& edges, const std::string& filename) {
    std::ofstream file(filename);
    file << "[\n";
    for (size_t i = 0; i < edges.size(); ++i) {
        file << "  " << edges[i].toJson() << (i < edges.size() - 1 ? "," : "") << "\n";
    }
    file << "]";
    file.close();
}