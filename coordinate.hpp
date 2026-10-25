#pragma once

namespace warehouse {

enum class Direction {
    Up, Down, Left, Right
};

struct Coordinate {
    int x = 0;
    int y = 0;

    Coordinate step(Direction dir) const;
    bool isNear(const Coordinate& other) const;
    int distance(const Coordinate& other) const;
};

}