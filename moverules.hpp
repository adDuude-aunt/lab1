#pragma once
#include "bot.hpp"
#include "map.hpp"

namespace warehouse {

class MovementRules {
    public:
        static bool canMove(const Robot& robot, const Coordinate& target, 
                            const WarehouseMap& map, const std::vector<Robot>& robots);
        static bool canLoad(const Robot& robot, const Cell& cell, int amount);
        static bool canUnload(const Robot& robot, const Cell& cell, int amount);
    };

}