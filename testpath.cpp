#include "path.hpp"
#include <gtest/gtest.h>
using namespace warehouse;

TEST(PathfindingTest, StraightPath) {
    WarehouseMap map(5, 5);
    auto path = Pathfinding::findPath({0, 0}, {3, 0}, map, {});
    EXPECT_EQ(path.size(), 4);
    EXPECT_EQ(path.front(), Coordinate(0, 0));
    EXPECT_EQ(path.back(), Coordinate(3, 0));
}

TEST(PathfindingTest, ObstacleAvoidance) {
    WarehouseMap map(5, 5);
    map.setCell({1, 0}, Cell(CellType::Wall));
    map.setCell({2, 0}, Cell(CellType::Wall));
    auto path = Pathfinding::findPath({0, 0}, {3, 0}, map, {});
    EXPECT_FALSE(path.empty());
    EXPECT_NE(path[1], Coordinate(1, 0));
}

TEST(PathfindingTest, NoPath) {
    WarehouseMap map(3, 3);
    map.setCell({1, 0}, Cell(CellType::Wall));
    map.setCell({1, 1}, Cell(CellType::Wall));
    map.setCell({1, 2}, Cell(CellType::Wall));
    auto path = Pathfinding::findPath({0, 0}, {2, 0}, map, {});
    EXPECT_TRUE(path.empty());
}