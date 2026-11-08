#include "graphs.hpp"
#include <gtest/gtest.h>

using namespace warehouse;

TEST(ChartTest, BarChartEmpty) {
    std::string result = ChartGenerator::renderBarChart("Test", {});
    EXPECT_NE(result.find("нет данных"), std::string::npos);
}

TEST(ChartTest, BarChartWithData) {
    std::vector<int> values = {1, 2, 3};
    std::string result = ChartGenerator::renderBarChart("Test", values);
    EXPECT_NE(result.find("Test"), std::string::npos);
    EXPECT_NE(result.find("3"), std::string::npos);
}

TEST(ChartTest, MultiLineChartEmpty) {
    std::string result = ChartGenerator::renderMultiLineChart("Test", {}, {});
    EXPECT_NE(result.find("нет данных"), std::string::npos);
}

TEST(ChartTest, MultiLineChartWithData) {
    std::vector<std::vector<int>> series = {{10, 20}, {15, 25}};
    std::vector<std::string> labels = {"R1", "R2"};
    std::string result = ChartGenerator::renderMultiLineChart("Test", series, labels);
    EXPECT_NE(result.find("R1"), std::string::npos);
    EXPECT_NE(result.find("R2"), std::string::npos);
}