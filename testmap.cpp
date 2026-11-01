#include "map.hpp"
#include <gtest/gtest.h>

using namespace warehouse;

TEST(WarehouseMapTest, DefaultConstruction) {
    WarehouseMap map(10, 8);
    EXPECT_EQ(map.width(), 10);
    EXPECT_EQ(map.height(), 8);
    EXPECT_EQ(map.pickupPosition(), Coordinate(0, 0));
    EXPECT_EQ(map.chargingPosition(), Coordinate(9, 7));
}

TEST(WarehouseMapTest, InvalidDimensions) {
    EXPECT_THROW(WarehouseMap(0, 10), Positionerror);
    EXPECT_THROW(WarehouseMap(10, 0), Positionerror);
    EXPECT_THROW(WarehouseMap(-1, 5), Positionerror);
}

TEST(WarehouseMapTest, CellAccess) {
    WarehouseMap map(5, 5);
    EXPECT_EQ(map.cellAt({0, 0}).type(), CellType::Pickup);
    EXPECT_EQ(map.cellAt({4, 4}).type(), CellType::C_Station);
    EXPECT_EQ(map.cellAt({2, 2}).type(), CellType::Empty);
}

TEST(WarehouseMapTest, OutOfBounds) {
    WarehouseMap map(5, 5);
    EXPECT_THROW(map.cellAt({5, 0}), Positionerror);
    EXPECT_THROW(map.cellAt({0, 5}), Positionerror);
    EXPECT_THROW(map.cellAt({-1, 0}), Positionerror);
}

TEST(WarehouseMapTest, SetCell) {
    WarehouseMap map(5, 5);
    map.setCell({2, 2}, Cell(CellType::Shelf, 10));
    EXPECT_EQ(map.cellAt({2, 2}).type(), CellType::Shelf);
    EXPECT_EQ(map.cellAt({2, 2}).stock(), 10);
    EXPECT_FALSE(map.isPassable({2, 2}));
}

TEST(WarehouseMapTest, SetPickupPosition) {
    WarehouseMap map(5, 5);
    map.setPickupPosition({3, 3});
    EXPECT_EQ(map.pickupPosition(), Coordinate(3, 3));
    EXPECT_EQ(map.cellAt({3, 3}).type(), CellType::Pickup);
    EXPECT_EQ(map.cellAt({0, 0}).type(), CellType::Empty);
}

TEST(WarehouseMapTest, SetChargingPosition) {
    WarehouseMap map(5, 5);
    map.setChargingPosition({1, 1});
    EXPECT_EQ(map.chargingPosition(), Coordinate(1, 1));
    EXPECT_EQ(map.cellAt({1, 1}).type(), CellType::C_Station);
    EXPECT_EQ(map.cellAt({4, 4}).type(), CellType::Empty);
}

TEST(WarehouseMapTest, IsPassable) {
    WarehouseMap map(5, 5);
    EXPECT_TRUE(map.isPassable({0, 0}));
    EXPECT_TRUE(map.isPassable({2, 2}));
    map.setCell({3, 3}, Cell(CellType::Wall));
    EXPECT_FALSE(map.isPassable({3, 3}));
    map.setCell({1, 1}, Cell(CellType::Shelf, 5));
    EXPECT_FALSE(map.isPassable({1, 1}));
}

TEST(WarehouseMapTest, IsValid) {
    WarehouseMap map(5, 5);
    EXPECT_TRUE(map.isValid({0, 0}));
    EXPECT_TRUE(map.isValid({4, 4}));
    EXPECT_FALSE(map.isValid({5, 0}));
    EXPECT_FALSE(map.isValid({0, 5}));
    EXPECT_FALSE(map.isValid({-1, 0}));
}