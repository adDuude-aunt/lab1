#pragma once

namespace warehouse {

enum class Direction {
    Up, Down, Left, Right
};

class Coordinate {
    int x_;
    int y_;
public:
    Coordinate(int x, int y);

    int x() const;
    int y() const;

    Coordinate step(Direction dir) const;
    bool isNear(const Coordinate& other) const;
    int distance(const Coordinate& other) const;

    bool operator==(const Coordinate& other) const;
};

}