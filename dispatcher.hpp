#pragma once
#include "bot.hpp"
#include "order.hpp"
#include "map.hpp"
#include "pathes.hpp"
#include "moverules.hpp"
#include "journal.hpp"
#include <vector>
#include <map>

namespace warehouse {

struct RobotTask {
    int robotId;
    int orderId;
    Coordinate shelfPos;
    std::vector<Coordinate> path;
    int pathIndex;
    int loadAmount;
    enum class Phase { ToShelf, Loading, ToPickup, Unloading, ToCharge, Idle, leaving };
    Phase phase;
    Phase resumePhase = Phase::Idle;
};

class Dispatcher {
private:
    std::map<int, RobotTask> tasks_;
    int chargeThreshold_ = 10;
    int chargeAmount_ = 5;

public:
    void assignOrders(std::vector<Order>& orders, std::vector<Robot>& robots,
                      const WarehouseMap& map, EventJournal& journal, int tick);
    void executeTick(std::vector<Robot>& robots, std::vector<Order>& orders,
                     WarehouseMap& map, EventJournal& journal, int tick);
    void sendToCharge(std::vector<Robot>& robots, const WarehouseMap& map,
                      EventJournal& journal, int tick);
    void assignTask(int robotId, int orderId, Coordinate shelfPos, int quantity, 
                          const WarehouseMap& map, const std::vector<Robot>& robots, 
                          EventJournal& journal, int tick);                  
};

}