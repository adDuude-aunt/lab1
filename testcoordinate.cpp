#include "coordinate.hpp"
#include <gtest/gtest.h>
using namespace warehouse;

TEST(CoordinateTest, StepMovesOneCell) {
    Coordinate c{5, 5};

    const auto up = c.step(Direction::Up);
    const auto down = c.step(Direction::Down);
    const auto left = c.step(Direction::Left);
    const auto right = c.step(Direction::Right);

    EXPECT_EQ(up.x, 5);
    EXPECT_EQ(up.y, 4);
    EXPECT_EQ(down.x, 5);
    EXPECT_EQ(down.y, 6);
    EXPECT_EQ(left.x, 4);
    EXPECT_EQ(left.y, 5);
    EXPECT_EQ(right.x, 6);
    EXPECT_EQ(right.y, 5);
}

TEST(CoordinateTest, StepDoesNotChangeCell) {
    Coordinate c{1, 1};
    c.step(Direction::Right);

    EXPECT_EQ(c.x, 1);
    EXPECT_EQ(c.y, 1);
}

TEST(CoordinateTest, NearCells) {
    Coordinate c{3, 3};
    Coordinate right{4, 3};
    Coordinate up{3, 2};
    Coordinate same{3, 3};
    Coordinate diagonal{4, 4};
    Coordinate far{5, 3};

    EXPECT_TRUE(c.isNear(right));
    EXPECT_TRUE(c.isNear(up));
    EXPECT_FALSE(c.isNear(same));
    EXPECT_FALSE(c.isNear(diagonal));
    EXPECT_FALSE(c.isNear(far));
}

TEST(CoordinateTest, Distance) {
    Coordinate a{0, 0};
    Coordinate b{2, 3};

    const auto d = a.distance(b);

    EXPECT_EQ(d, 5);
    EXPECT_EQ(b.distance(a), 5);
    EXPECT_EQ(a.distance(a), 0);
}