#include "coordinate.hpp"

namespace warehouse {

Coordinate::Coordinate(int x, int y) : x_(x), y_(y) {
}

int Coordinate::x() const {
    return x_;
}

int Coordinate::y() const {
    return y_;
}

Coordinate Coordinate::step(Direction dir) const {
    if (dir == Direction::Up)
        return {x_, y_ - 1};

    if (dir == Direction::Down)
        return {x_, y_ + 1};

    if (dir == Direction::Left)
        return {x_ - 1, y_};

    return {x_ + 1, y_};
}

bool Coordinate::isNear(const Coordinate& other) const {
    return distance(other) == 1;
}

int Coordinate::distance(const Coordinate& other) const {
    int dx = x_ - other.x_;
    int dy = y_ - other.y_;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    return dx + dy;
}

bool Coordinate::operator==(const Coordinate& other) const {
    return x_ == other.x_ && y_ == other.y_;
}

}