#include "cell.hpp"
#include <gtest/gtest.h>

using namespace warehouse;

TEST(CellTest, Construction) {
    Cell c;
    EXPECT_EQ(c.type(), CellType::Empty);
    EXPECT_EQ(c.stock(), 0);
    EXPECT_TRUE(c.isPassable());
}

TEST(CellTest, Shelf) {
    Cell c(CellType::Shelf, 10);
    EXPECT_TRUE(c.isShelf());
    EXPECT_FALSE(c.isPassable());
    EXPECT_EQ(c.stock(), 10);
}

TEST(CellTest, Mutation) {
    Cell c(CellType::Shelf, 10);
    c.addStock(5);
    EXPECT_EQ(c.stock(), 15);
    
    c.removeStock(3);
    EXPECT_EQ(c.stock(), 12);
}

TEST(CellTest, S_Invariants) {
    Cell c(CellType::Shelf, 10);
    
    EXPECT_THROW(c.removeStock(15), CellState);
    EXPECT_THROW(c.addStock(-5), CellState);
    EXPECT_THROW(c.removeStock(-1), CellState);
}

TEST(CellTest, T_Invariants) {
    EXPECT_THROW(Cell(CellType::Empty, 5), CellState);
    EXPECT_THROW(Cell(CellType::Wall, 5), CellState);
    EXPECT_THROW(Cell(CellType::Pickup, 5), CellState);
}

TEST(CellTest, Rules) {
    EXPECT_TRUE(Cell(CellType::Empty).isPassable());
    EXPECT_TRUE(Cell(CellType::Pickup).isPassable());
    EXPECT_TRUE(Cell(CellType::C_Station).isPassable());
    EXPECT_FALSE(Cell(CellType::Wall).isPassable());
    EXPECT_FALSE(Cell(CellType::Shelf).isPassable());
}

TEST(CellTest, Equality) {
    Cell c1(CellType::Shelf, 10);
    Cell c2(CellType::Shelf, 10);
    Cell c3(CellType::Shelf, 5);
    
    EXPECT_EQ(c1, c2);
    EXPECT_NE(c1, c3);
}