#include "order.hpp"
#include <gtest/gtest.h>

using namespace warehouse;

TEST(OrderTest, Construction) {
    Order o(111, {6, 7}, 14);
    EXPECT_EQ(o.id(), 111);
    EXPECT_EQ(o.shelfPosition(), Coordinate(6, 7));
    EXPECT_EQ(o.quantity(), 14);
    EXPECT_EQ(o.status(), OrderStatus::Pending);
    EXPECT_EQ(o.assignedRobotId(), -1);
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
    o.assignToRobot(46);
    EXPECT_EQ(o.assignedRobotId(), 46);
    EXPECT_EQ(o.status(), OrderStatus::Assigned);
    EXPECT_TRUE(o.isAssigned());
}

TEST(OrderTest, NegativeRobot) {
    Order o(111, {6, 7}, 14);
    EXPECT_THROW(o.assignToRobot(-1), StateError);
}

TEST(OrderTest, AssignAssigned) {
    Order o(111, {6, 7}, 14);
    o.assignToRobot(46);
    EXPECT_THROW(o.assignToRobot(90), StateError);
}

TEST(OrderTest, Progress) {
    Order o(111, {6, 7}, 14);
    o.assignToRobot(46);
    o.markInProgress();
    EXPECT_EQ(o.status(), OrderStatus::InProgress);
}

TEST(OrderTest, noAssignment) {
    Order o(111, {6, 7}, 14);
    EXPECT_THROW(o.InProgress(), StateError);
}

TEST(OrderTest, Completed) {
    Order o(111, {6, 7}, 14);
    o.assignToRobot(46);
    o.markInProgress();
    o.markCompleted();
    EXPECT_EQ(o.status(), OrderStatus::Completed);
    EXPECT_TRUE(o.isCompleted());
}

TEST(OrderTest, noProgress) {
    Order o(111, {6, 7}, 14);
    o.assignToRobot(5);
    EXPECT_THROW(o.markCompleted(), StateError);
}

TEST(OrderTest, Cancelled) {
    Order o(111, {6, 7}, 14);
    o.markCancelled();
    EXPECT_EQ(o.status(), OrderStatus::Cancelled);
    EXPECT_TRUE(o.isCancelled());
}

TEST(OrderTest, CancelAssigned) {
    Order o(111, {6, 7}, 14);
    o.assignToRobot(46);
    o.markCancelled();
    EXPECT_TRUE(o.isCancelled());
}

TEST(OrderTest, CancelCompleted) {
    Order o(111, {6, 7}, 14);
    o.assignToRobot(46);
    o.markInProgress();
    o.markCompleted();
    EXPECT_THROW(o.markCancelled(), StateError);
}

TEST(OrderTest, LowStock) {
    Order o(111, {6, 7}, 14);
    o.LowStock();
    EXPECT_EQ(o.status(), OrderStatus::LowStock);
}

TEST(OrderTest, PendingLowStock) {
    Order o(111, {6, 7}, 14);
    o.assignToRobot(46);
    EXPECT_THROW(o.LowStock(), StateError);
}

TEST(OrderTest, Equality) {
    Order o1(111, {6, 7}, 14);
    Order o2(111, {5, 2}, 92);
    Order o3(222, {6, 7}, 14);
    EXPECT_EQ(o1, o2);
    EXPECT_NE(o1, o3);
}
