#include "dispatcher.hpp"
#include <gtest/gtest.h>

using namespace warehouse;

TEST(DispatcherTest, AssignOrder) {
    WarehouseMap map(10, 10);
    map.setCell({3, 3}, Cell(CellType::Shelf, 50));
    std::vector<Robot> robots;
    robots.emplace_back(1, Coordinate{1, 1}, 100, 10);
    std::vector<Order> orders;
    orders.emplace_back(1, Coordinate{3, 3}, 5);
    EventJournal journal;
    Dispatcher disp;
    
    disp.assignOrders(orders, robots, map, journal, 1);
    EXPECT_EQ(orders[0].status(), OrderStatus::InProgress);
    EXPECT_EQ(orders[0].botId(), 1);
}

TEST(DispatcherTest, InsufficientStock) {
    WarehouseMap map(10, 10);
    map.setCell({3, 3}, Cell(CellType::Shelf, 2));
    std::vector<Robot> robots;
    robots.emplace_back(1, Coordinate{1, 1}, 20, 10);
    std::vector<Order> orders;
    orders.emplace_back(1, Coordinate{3, 3}, 5);
    EventJournal journal;
    Dispatcher disp;
    
    disp.assignOrders(orders, robots, map, journal, 1);
    EXPECT_EQ(orders[0].status(), OrderStatus::LowStock);
}