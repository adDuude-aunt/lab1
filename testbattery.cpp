#include "battery.hpp"
#include <gtest/gtest.h>
#include <string>
using namespace warehouse;

TEST(BatteryTest, StartLevel) {
    const Battery b{45, 100};

    EXPECT_EQ(b.level(), 45);
    EXPECT_EQ(b.maxLevel(), 100);
}

TEST(BatteryTest, SpendReducesLevel) {
    Battery b{50, 100};

    EXPECT_EQ(b.level(), 50);
}

TEST(BatteryTest, SpendMore) {
    Battery b{5, 100};

    EXPECT_THROW(b.spend(20), LackEnergy);

    EXPECT_EQ(b.level(), 5);
}

TEST(BatteryTest, Zero) {
    Battery b{90, 100};

    b.spend(90);

    EXPECT_EQ(b.level(), 0);
    EXPECT_FALSE(b.hasEnough(1));
}

TEST(BatteryTest, HasEnough) {
    const Battery b{52, 100};  

    EXPECT_TRUE(b.hasEnough(52));
    EXPECT_FALSE(b.hasEnough(67));
}

TEST(BatteryTest, Max) {
    Battery b{90, 100};

    b.charge(50);

    EXPECT_EQ(b.level(), 100);
    EXPECT_TRUE(b.isFull());
}

TEST(BatteryTest, Equality) {
    const Battery left{40, 100};
    const Battery same{40, 100};
    const Battery other{41, 100};

    EXPECT_TRUE(left == same);
    EXPECT_FALSE(left == other);
}

struct BatteryCase {
    const char* name;
    int level;
    int maxLevel;
};

class BatteryTestP : public ::testing::TestWithParam<BatteryCase> {};

TEST_P(BatteryTestP, InvalidRangeIsRejected) {
    const auto& p = GetParam();

    EXPECT_THROW((Battery{p.level, p.maxLevel}), energyRange);
}

const BatteryCase rejected_cases[] = {
    {"Negative", -1, 100},
    {"OverMax", 101, 100},
    {"ZeroMax", 5, 0},
    {"NegativeMax", 5, -10},
};

INSTANTIATE_TEST_SUITE_P(
    InvalidBatteryCases, 
    BatteryTestP, 
    ::testing::ValuesIn(rejected_cases)
);

class NegativeCostTest : public ::testing::TestWithParam<int> {};

TEST_P(NegativeCostTest, NegativeCost) {
    Battery b{50, 100};

    EXPECT_THROW(b.spend(GetParam()), energyCost); 
    EXPECT_THROW(b.charge(GetParam()), energyCost);

    EXPECT_EQ(b.level(), 50);
}

INSTANTIATE_TEST_SUITE_P(Boundaries, NegativeCostTest,
    ::testing::Values(-1, -100));