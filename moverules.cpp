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
    Direction parse_direction(const std::string& s) {
    if (s == "up") return Direction::Up;
    if (s == "down") return Direction::Down;
    if (s == "left") return Direction::Left;
    if (s == "right") return Direction::Right;
    throw std::invalid_argument("Неверное направление");
}

std::string direction_to_string(Direction d) {
    switch (d) {
        case Direction::Up:    return "up";
        case Direction::Down:  return "down";
        case Direction::Left:  return "left";
        case Direction::Right: return "right";
    }
    return "?";
}

std::vector<AdjacentShelf> find_adjacent_shelves(Robot& robot, const WarehouseMap& map) {
    std::vector<AdjacentShelf> result;
    Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    for (auto d : dirs) {
        Coordinate neighbor = robot.position().step(d);
        if (map.isValid(neighbor)) {
            Cell& cell = const_cast<Cell&>(map.cellAt(neighbor));
            if (cell.isShelf() && cell.stock() > 0) {
                result.push_back({d, &cell, neighbor});
            }
        }
    }
    return result;
}
}