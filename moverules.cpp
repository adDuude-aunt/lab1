#include "moverules.hpp"

namespace warehouse {

    bool MovementRules::canMove(const Robot& robot, const Coordinate& target, 
                                const WarehouseMap& map, const std::vector<Robot>& robots) {
        if (!map.isValid(target) || !map.isPassable(target)) return false;
        if (!robot.able2move()) return false;
        for (const auto& r : robots) {
            if (r.id() != robot.id() && r.position() == target) return false;
        }
        return true;
    }

    bool MovementRules::canLoad(const Robot& robot, const Cell& cell, int amount) {
        if (!cell.isShelf()) return false;
        if (cell.stock() < amount) return false;
        if (!robot.Carry(amount)) return false;
        return true;
    }

    bool MovementRules::canUnload(const Robot& robot, const Cell& cell, int amount) {
        if (!cell.isPickup()) return false;
        if (robot.deliver() < amount) return false;
        return true;
    }
}