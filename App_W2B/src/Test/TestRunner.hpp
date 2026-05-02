#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

class TestRunner
{
public:
    struct ExpectedOutput
    {
        int shapeCount = -1; // -1 = not checked
        int edgeCount = -1;  // -1 = not checked
        std::vector<std::string> shapeTypes; // empty = not checked
    };

    struct TestCase
    {
        std::string name;
        std::string imagePath; // relative to appRoot or absolute
        ExpectedOutput expected;
    };

    struct TestResult
    {
        std::string name;
        bool passed = true;
        std::vector<std::string> failures;
        int actualShapeCount = 0;
        int actualEdgeCount = 0;
    };

    // Load test cases from a JSON file
    static std::vector<TestCase> loadTests(const std::string &testsJsonPath);

    // Run all test cases and return results
    static std::vector<TestResult> runAll(const std::vector<TestCase> &tests,
                                          const std::filesystem::path &appRoot);

    // Print a human-readable summary of all results
    static void printSummary(const std::vector<TestResult> &results);
};
