#pragma once
#include "map.hpp"
#include "bot.hpp"
#include "order.hpp"
#include "dispatcher.hpp"
#include "journal.hpp"
#include <vector>
#include <memory>
#include <iostream>

namespace warehouse {

struct TickStats {
    int tick;
    int pendingOrders;
    int completedOrders;
    std::vector<int> robotCharges;
};

class Simulator {
    WarehouseMap map_;
    std::vector<Robot> robots_;
    std::vector<Order> orders_;
    Dispatcher dispatcher_;
    EventJournal journal_;
    int currentTick_;
    int nextOrderId_;
    std::vector<TickStats> stats_;

public:
    Simulator(WarehouseMap map, std::vector<Robot> robots);
    
    void tick();
    int currentTick() const;
    
    void createOrder(Coordinate shelfPos, int quantity);
    void assignOrderToRobot(int orderId, int robotId);
    void moveRobot(int robotId, Direction dir);
    void loadCargo(int robotId, int amount);
    void unloadCargo(int robotId, int amount);
    void sendRobotToCharge(int robotId);
    
    const WarehouseMap& map() const;
    const std::vector<Robot>& robots() const;
    const std::vector<Order>& orders() const;
    const EventJournal& journal() const;
    const std::vector<TickStats>& stats() const;
    
    void saveLog(const std::string& filename) const;
    void loadLog(const std::string& filename);

private:
    Robot* findRobot(int id);
    Order* findOrder(int id);
    void collectStats();
};

int read_int(const std::string& prompt, int min_val, int max_val);

}