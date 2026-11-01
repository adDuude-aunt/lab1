#pragma once
#include "coordinate.hpp"
#include "cell.hpp"
#include "domainerror.hpp"
#include <vector>

namespace warehouse {

struct Positionerror : DomainError {
    using DomainError::DomainError;
};

struct Placementerror : DomainError {
    using DomainError::DomainError;
};

class WarehouseMap {
private:
    int width_;
    int height_;
    std::vector<std::vector<Cell>> grid_;
    Coordinate pickupPos_;
    Coordinate chargingPos_;

    void validatePosition(const Coordinate& pos) const;
    void validateCellForPlacement(const Coordinate& pos, const Cell& cell) const;

public:
    WarehouseMap(int width, int height);

    int width() const;
    int height() const;

    const Cell& cellAt(const Coordinate& pos) const;
    Cell& cellAt(const Coordinate& pos);
    void setCell(const Coordinate& pos, const Cell& cell);

    bool isValid(const Coordinate& pos) const;
    bool isPassable(const Coordinate& pos) const;

    Coordinate pickupPosition() const;
    void setPickupPosition(const Coordinate& pos);

    Coordinate chargingPosition() const;
    void setChargingPosition(const Coordinate& pos);
};

} 