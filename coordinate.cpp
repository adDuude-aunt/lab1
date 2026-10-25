#include "coordinate.hpp"

namespace warehouse {

Coordinate Coordinate::step(Direction dir) const {
    if (dir == Direction::Up)
        return {x, y - 1};

    if (dir == Direction::Down)
        return {x, y + 1};

    if (dir == Direction::Left)
        return {x - 1, y};

    return {x + 1, y};
}

bool Coordinate::isNear(const Coordinate& other) const {
    return distance(other) == 1;
}

int Coordinate::distance(const Coordinate& other) const {
    int dx = x - other.x;
    int dy = y - other.y;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    return dx + dy;
}

}