#include "moverules.hpp"
#include <gtest/gtest.h>
using namespace warehouse;

TEST(MovementRulesTest, CanMoveBasic) {
    WarehouseMap map(5, 5);
    Robot r(1, {0, 0}, 20, 10);
    EXPECT_TRUE(MovementRules::canMove(r, {2, 0}, map, {}));
    EXPECT_FALSE(MovementRules::canMove(r, {5, 0}, map, {})); 
}

TEST(MovementRulesTest, Collision) {
    WarehouseMap map(5, 5);
    Robot r1(1, {0, 0}, 20, 10);
    Robot r2(2, {1, 0}, 20, 10);
    EXPECT_FALSE(MovementRules::canMove(r1, {1, 0}, map, {r2}));
}

TEST(MovementRulesTest, LoadUnload) {
    Robot r(1, {0, 0}, 20, 10);
    Cell shelf(CellType::Shelf, 5);
    Cell pickup(CellType::Pickup, 0);
    
    EXPECT_TRUE(MovementRules::canLoad(r, shelf, 3));
    EXPECT_FALSE(MovementRules::canLoad(r, shelf, 6));
    EXPECT_FALSE(MovementRules::canLoad(r, pickup, 3));
    
    r.load(3);
    EXPECT_TRUE(MovementRules::canUnload(r, pickup, 3));
    EXPECT_FALSE(MovementRules::canUnload(r, pickup, 4));
}