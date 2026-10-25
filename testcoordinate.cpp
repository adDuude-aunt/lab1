#include "coordinate.hpp"
#include <gtest/gtest.h>
using namespace warehouse;

TEST(CoordinateTest, StepMovesOneCell) {
    const Coordinate c{5, 5};                   

    const auto up = c.step(Direction::Up);        
    const auto right = c.step(Direction::Right);

    EXPECT_EQ(up.x(), 5);                       
    EXPECT_EQ(up.y(), 4);
    EXPECT_EQ(right.x(), 6);
    EXPECT_EQ(right.y(), 5);
}

TEST(CoordinateTest, StepDoesNotChangeCell) {
    const Coordinate c{1, 1};                     

    c.step(Direction::Right);                     

    EXPECT_EQ(c.x(), 1);                          
    EXPECT_EQ(c.y(), 1);
}

TEST(CoordinateTest, NearCells) {
    const Coordinate c{3, 3};                     
    const Coordinate right{4, 3};
    const Coordinate diagonal{4, 4};
    const Coordinate far{5, 3};

    EXPECT_TRUE(c.isNear(right));                
    EXPECT_FALSE(c.isNear(diagonal));
    EXPECT_FALSE(c.isNear(far));
    EXPECT_FALSE(c.isNear(c));
}

TEST(CoordinateTest, Distance) {
    const Coordinate a{0, 0};   
    const Coordinate b{2, 3};

    const auto d = a.distance(b);              

    EXPECT_EQ(d, 5);
    EXPECT_EQ(b.distance(a), 5);
    EXPECT_EQ(a.distance(a), 0);
}

TEST(CoordinateTest, EqualityByContent) {
    const Coordinate left{1, 2}; 
    const Coordinate same{1, 2};
    const Coordinate swapped{2, 1};

    EXPECT_TRUE(left == same);
    EXPECT_FALSE(left == swapped);
}

class StepTest : public ::testing::TestWithParam<Direction> {};

TEST_P(StepTest, LandsOnNeighbourCell) {
    const Coordinate c{4, 4};

    const auto next = c.step(GetParam());

    EXPECT_TRUE(c.isNear(next));
    EXPECT_EQ(c.distance(next), 1);
}

INSTANTIATE_TEST_SUITE_P(Directions, StepTest,
    ::testing::Values(Direction::Up, Direction::Down,
                      Direction::Left, Direction::Right));