#include "TestRunner.hpp"
#include "../Detector/Detector.hpp"
#include "../ImageWork/ImageLoader.hpp"
#include "../ImageWork/ImageMask.hpp"
#include "../JsonExport/JsonExporter.hpp"
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace Color {
    constexpr const char* RESET  = "\033[0m";
    constexpr const char* BOLD   = "\033[1m";
    constexpr const char* DIM    = "\033[2m";
    constexpr const char* GREEN  = "\033[32m";
    constexpr const char* RED    = "\033[31m";
    constexpr const char* CYAN   = "\033[36m";
}

std::vector<TestRunner::TestCase> TestRunner::loadTests(const std::string &testsJsonPath)
{
    std::vector<TestCase> tests;
    std::ifstream file(testsJsonPath);
    if (!file.is_open())
    {
        std::cerr << "[TestRunner] Could not open tests file: " << testsJsonPath << std::endl;
        return tests;
    }

    json data;
    file >> data;

    for (const auto &item : data)
    {
        TestCase tc;
        tc.name = item.value("name", "unnamed");
        tc.imagePath = item.value("imagePath", "");

        if (item.contains("expected"))
        {
            const auto &exp = item["expected"];
            tc.expected.shapeCount = exp.value("shapeCount", -1);
            tc.expected.edgeCount = exp.value("edgeCount", -1);
            if (exp.contains("shapeTypes"))
            {
                for (const auto &t : exp["shapeTypes"])
                    tc.expected.shapeTypes.push_back(t.get<std::string>());
            }
        }

        tests.push_back(tc);
    }

    return tests;
}

std::vector<TestRunner::TestResult> TestRunner::runAll(const std::vector<TestCase> &tests,
                                                        const std::filesystem::path &appRoot)
{
    std::vector<TestResult> results;

    for (const auto &tc : tests)
    {
        TestResult result;
        result.name = tc.name;

        std::cout << "\n" << Color::CYAN << Color::BOLD << "[TEST] Running: " << tc.name << Color::RESET << std::endl;

        // Resolve image path (relative to appRoot if not absolute)
        std::filesystem::path imgPath(tc.imagePath);
        if (imgPath.is_relative())
            imgPath = appRoot / imgPath;

        cv::Mat image = ImageLoader::loadImage(imgPath.string(),
                                               (appRoot / "json" / "imageConfig.json").string());
        if (image.empty())
        {
            result.passed = false;
            result.failures.push_back("Could not load image: " + imgPath.string());
            results.push_back(result);
            continue;
        }

        // Detect shapes
        Detector detector(image);
        std::vector<Shape> shapes = detector.detectShapes(image);
        JsonExporter::saveShapes(shapes, (appRoot / "json" / "detectedShapes.json").string());

        // Detect edges
        cv::Mat mask = ImageMask::createMask(image);
        std::vector<Edge> edges = detector.detectEdges(mask, shapes);
        JsonExporter::saveEdges(edges, (appRoot / "json" / "detectedEdges.json").string());

        result.actualShapeCount = static_cast<int>(shapes.size());
        result.actualEdgeCount = static_cast<int>(edges.size());

        std::cout << Color::DIM << "  Detected shapes: " << result.actualShapeCount
                  << "  edges: " << result.actualEdgeCount << Color::RESET << std::endl;

        // --- Compare with expected ---
        if (tc.expected.shapeCount != -1 && result.actualShapeCount != tc.expected.shapeCount)
        {
            result.passed = false;
            result.failures.push_back(
                "shapeCount: expected " + std::to_string(tc.expected.shapeCount) +
                ", got " + std::to_string(result.actualShapeCount));
        }

        if (tc.expected.edgeCount != -1 && result.actualEdgeCount != tc.expected.edgeCount)
        {
            result.passed = false;
            result.failures.push_back(
                "edgeCount: expected " + std::to_string(tc.expected.edgeCount) +
                ", got " + std::to_string(result.actualEdgeCount));
        }

        if (!tc.expected.shapeTypes.empty())
        {
            // Count type occurrences in detected shapes
            std::map<std::string, int> detectedTypeCounts;
            for (const auto &s : shapes)
                detectedTypeCounts[s.type]++;

            std::map<std::string, int> expectedTypeCounts;
            for (const auto &t : tc.expected.shapeTypes)
                expectedTypeCounts[t]++;

            for (const auto &[type, count] : expectedTypeCounts)
            {
                int actual = detectedTypeCounts.count(type) ? detectedTypeCounts[type] : 0;
                if (actual != count)
                {
                    result.passed = false;
                    result.failures.push_back(
                        "shapeType '" + type + "': expected " + std::to_string(count) +
                        ", got " + std::to_string(actual));
                }
            }
        }

        results.push_back(result);
    }

    return results;
}

void TestRunner::printSummary(const std::vector<TestResult> &results)
{
    int passed = 0, failed = 0;
    const int width = 40;

    std::cout << "\n" << Color::BOLD << std::string(60, '=') << Color::RESET << std::endl;
    std::cout << Color::BOLD << "  TEST SUMMARY" << Color::RESET << std::endl;
    std::cout << Color::BOLD << std::string(60, '=') << Color::RESET << std::endl;

    for (const auto &r : results)
    {
        const char* statusColor = r.passed ? Color::GREEN : Color::RED;
        std::string status = r.passed ? "PASS" : "FAIL";
        std::cout << std::left << std::setw(width) << r.name
                  << "  " << statusColor << Color::BOLD << "[" << status << "]" << Color::RESET
                  << Color::DIM << "  (shapes: " << r.actualShapeCount
                  << ", edges: " << r.actualEdgeCount << ")" << Color::RESET << std::endl;

        for (const auto &f : r.failures)
            std::cout << Color::RED << "      - " << f << Color::RESET << std::endl;

        (r.passed ? passed : failed)++;
    }

    std::cout << Color::DIM << std::string(60, '-') << Color::RESET << std::endl;
    std::cout << Color::GREEN << Color::BOLD << "  Passed: " << passed << " / " << results.size() << Color::RESET << std::endl;
    if (failed > 0)
        std::cout << Color::RED << Color::BOLD << "  Failed: " << failed << Color::RESET << std::endl;
    std::cout << Color::BOLD << std::string(60, '=') << Color::RESET << std::endl;
}
