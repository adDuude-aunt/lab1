#include "bot.hpp"
#include <gtest/gtest.h>

using namespace warehouse;

TEST(botTest, Construction) {
    Robot r(67, {0, 0}, 17, 7);
    EXPECT_EQ(r.id(), 67);
    EXPECT_EQ(r.position(), Coordinate(0, 0));
    EXPECT_EQ(r.deliver(), 0);
    EXPECT_EQ(r.capacity(), 7);
    EXPECT_EQ(r.freeCapacity(), 7);
    EXPECT_FALSE(r.isCharging());
    EXPECT_EQ(r.battery().level(), 17);
}

TEST(botTest, Id) {
    EXPECT_THROW(Robot(-67, {0, 0}, 17, 7), StateError);
}

TEST(botTest, Capacity) {
    EXPECT_THROW(Robot(67, {0, 0}, 17, 0), CapacityError);
    EXPECT_THROW(Robot(67, {0, 0}, 17, -52), CapacityError);
}

TEST(RobotTest, Load) {
    Robot r(67, {0, 0}, 17, 7);
    r.load(5);
    EXPECT_EQ(r.deliver(), 5);
    EXPECT_EQ(r.freeCapacity(), 2);
}

TEST(RobotTest, Overload) {
    Robot r(67, {0, 0}, 17, 7);
    r.load(7);
    EXPECT_THROW(r.load(1), CapacityError);
}

TEST(RobotTest, LoadNegative) {
    Robot r(67, {0, 0}, 17, 7);
    EXPECT_THROW(r.load(-1), DeliverError);
}

TEST(RobotTest, Unload) {
    Robot r(67, {0, 0}, 17, 7);
    r.load(5);
    r.unload(3);
    EXPECT_EQ(r.deliver(), 2);
}

TEST(RobotTest, UnloadMore) {
    Robot r(67, {0, 0}, 17, 7);
    r.load(5);
    EXPECT_THROW(r.unload(6), DeliverError);
}

TEST(RobotTest, UnloadNegative) {
    Robot r(67, {0, 0}, 17, 7);
    EXPECT_THROW(r.unload(-1), DeliverError);
}

TEST(RobotTest, MoveCost) {
    Robot r(67, {0, 0}, 17, 7);
    EXPECT_EQ(r.moveCost(), 1);
    r.load(3);
    EXPECT_EQ(r.moveCost(), 4);
}

TEST(RobotTest, MoveTo) {
    Robot r(67, {0, 0}, 17, 7);
    r.moveTo({1, 0});
    EXPECT_EQ(r.position(), Coordinate(1, 0));
    EXPECT_EQ(r.battery().level(), 16);
}

TEST(RobotTest, MoveCharging) {
    Robot r(67, {0, 0}, 17, 7);
    r.setCharging(true);
    EXPECT_THROW(r.moveTo({1, 0}), StateError);
}

TEST(RobotTest, noEnergy) {
    Robot r(67, {0, 0}, 1, 7);
    r.load(1); // moveCost = 2
    EXPECT_THROW(r.moveTo({1, 0}), StateError);
}

TEST(RobotTest, Charge) {
    Robot r(67, {0, 0}, 17, 7);
    r.spendEnergy(10);
    EXPECT_EQ(r.battery().level(), 7);
    r.setCharging(true);
    r.charge(5);
    EXPECT_EQ(r.battery().level(), 12);
}

TEST(RobotTest, ChargeOutOfPlace) {
    Robot r(67, {0, 0}, 17, 7);
    EXPECT_THROW(r.charge(5), StateError);
}

TEST(RobotTest, Carry) {
    Robot r(67, {0, 0}, 17, 7);
    EXPECT_TRUE(r.Carry(7));
    EXPECT_FALSE(r.Carry(8));
    r.load(5);
    EXPECT_TRUE(r.Carry(2));
    EXPECT_FALSE(r.Carry(3));
}

TEST(RobotTest, Equality) {
    Robot r1(67, {0, 0}, 17, 7);
    Robot r2(67, {4, 2}, 44, 66);
    Robot r3(52, {0, 0}, 17, 7);
    EXPECT_EQ(r1, r2);
    EXPECT_NE(r1, r3);
}