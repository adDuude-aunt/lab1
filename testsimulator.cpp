#include "simulator.hpp"
#include <gtest/gtest.h>
#include <filesystem>
using namespace warehouse;

TEST(SimulatorTest, TickAndStats) {
    WarehouseMap map(10, 10);
    map.setCell({3, 3}, Cell(CellType::Shelf, 50));
    std::vector<Robot> robots;
    robots.emplace_back(1, Coordinate{1, 1}, 20, 10);
    Simulator sim(std::move(map), std::move(robots));
    
    sim.createOrder({3, 3}, 5);
    sim.tick();
    EXPECT_EQ(sim.currentTick(), 1);
    EXPECT_FALSE(sim.stats().empty());
    EXPECT_EQ(sim.stats()[0].pendingOrders, 1);
}

TEST(SimulatorTest, SaveLoadLog) {
    WarehouseMap map(5, 5);
    map.setCell({2, 2}, Cell(CellType::Shelf, 10)); 
    Simulator sim(std::move(map), {});
    sim.createOrder({2, 2}, 5);
    sim.saveLog("sim_log.txt");
    
    Simulator sim2(WarehouseMap(5, 5), {});
    sim2.loadLog("sim_log.txt");
    EXPECT_FALSE(sim2.journal().events().empty());
    std::filesystem::remove("sim_log.txt");
}