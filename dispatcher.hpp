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
    std::vector<Coordinate> path;
    int pathIndex;
    int orderId;
    enum class Phase { ToShelf, Loading, ToPickup, Unloading, ToCharge, Idle };
    Phase phase;
    int loadAmount;
};

class Dispatcher {
    std::map<int, RobotTask> tasks_;
    int chargeThreshold_ = 5;
    int chargeAmount_ = 3;

public:
    void assignOrders(std::vector<Order>& orders, std::vector<Robot>& robots, 
                      const WarehouseMap& map, EventJournal& journal, int tick);
    void executeTick(std::vector<Robot>& robots, std::vector<Order>& orders, 
                     WarehouseMap& map, EventJournal& journal, int tick);
    void sendToCharge(std::vector<Robot>& robots, const WarehouseMap& map, 
                      EventJournal& journal, int tick);
};

}