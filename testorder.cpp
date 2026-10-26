#include "order.hpp"
#include <gtest/gtest.h>

using namespace warehouse;

TEST(OrderTest, Construction) {
    Order o(111, {6, 7}, 14);
    EXPECT_EQ(o.id(), 111);
    EXPECT_EQ(o.shelfPosition(), Coordinate(6, 7));
    EXPECT_EQ(o.quantity(), 14);
    EXPECT_EQ(o.status(), OrderStatus::Pending);
    EXPECT_EQ(o.botId(), -1);
    EXPECT_TRUE(o.isPending());
}

TEST(OrderTest, Id) {
    EXPECT_THROW(Order(-111, {6, 7}, 14), StateError);
}

TEST(OrderTest, Quantity) {
    EXPECT_THROW(Order(111, {6, 7}, 0), QuantityError);
    EXPECT_THROW(Order(111, {6, 7}, -4), QuantityError);
}

TEST(OrderTest, AssignToRobot) {
    Order o(111, {6, 7}, 14);
    o.AttachedRobot(46);
    EXPECT_EQ(o.botId(), 46);
    EXPECT_EQ(o.status(), OrderStatus::Setted);
    EXPECT_TRUE(o.isSetted());
}

TEST(OrderTest, NegativeRobot) {
    Order o(111, {6, 7}, 14);
    EXPECT_THROW(o.AttachedRobot(-1), StateError);
}

TEST(OrderTest, AssignAssigned) {
    Order o(111, {6, 7}, 14);
    o.AttachedRobot(46);
    EXPECT_THROW(o.AttachedRobot(90), StateError);
}

TEST(OrderTest, Progress) {
    Order o(111, {6, 7}, 14);
    o.AttachedRobot(46);
    o.InProgress();
    EXPECT_EQ(o.status(), OrderStatus::InProgress);
}

TEST(OrderTest, noAssignment) {
    Order o(111, {6, 7}, 14);
    EXPECT_THROW(o.InProgress(), StateError);
}

TEST(OrderTest, Completed) {
    Order o(111, {6, 7}, 14);
    o.AttachedRobot(46);
    o.InProgress();
    o.Completed();
    EXPECT_EQ(o.status(), OrderStatus::Completed);
    EXPECT_TRUE(o.isCompleted());
}

TEST(OrderTest, noProgress) {
    Order o(111, {6, 7}, 14);
    o.AttachedRobot(5);
    EXPECT_THROW(o.Completed(), StateError);
}

TEST(OrderTest, Cancelled) {
    Order o(111, {6, 7}, 14);
    o.Cancelled();
    EXPECT_EQ(o.status(), OrderStatus::Cancelled);
    EXPECT_TRUE(o.isCancelled());
}

TEST(OrderTest, CancelAssigned) {
    Order o(111, {6, 7}, 14);
    o.AttachedRobot(46);
    o.Cancelled();
    EXPECT_TRUE(o.isCancelled());
}

TEST(OrderTest, CancelCompleted) {
    Order o(111, {6, 7}, 14);
    o.AttachedRobot(46);
    o.InProgress();
    o.Completed();
    EXPECT_THROW(o.Cancelled(), StateError);
}

TEST(OrderTest, LowStock) {
    Order o(111, {6, 7}, 14);
    o.LowStock();
    EXPECT_EQ(o.status(), OrderStatus::LowStock);
}

TEST(OrderTest, PendingLowStock) {
    Order o(111, {6, 7}, 14);
    o.AttachedRobot(46);
    EXPECT_THROW(o.LowStock(), StateError);
}

TEST(OrderTest, Equality) {
    Order o1(111, {6, 7}, 14);
    Order o2(111, {5, 2}, 92);
    Order o3(222, {6, 7}, 14);
    EXPECT_EQ(o1, o2);
    EXPECT_NE(o1, o3);
}
