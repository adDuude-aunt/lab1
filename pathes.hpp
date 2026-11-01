#pragma once
#include "coordinate.hpp"
#include "map.hpp"
#include <vector>

namespace warehouse {

class Pathfinding {
public:
    static std::vector<Coordinate> findPath(const Coordinate& start, const Coordinate& end, 
                                            const WarehouseMap& map, 
                                            const std::vector<Coordinate>& obstacles);
};

}