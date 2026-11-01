#include "map.hpp"

namespace warehouse {

WarehouseMap::WarehouseMap(int width, int height): width_(width), height_(height),
      grid_(height, std::vector<Cell>(width, Cell(CellType::Empty))),
      pickupPos_(0, 0), chargingPos_(width - 1, height - 1) {
    if (width <= 0 || height <= 0) {
        throw Positionerror("Map dimensions can`t be negative");
    }

    grid_[0][0] = Cell(CellType::Pickup);
    grid_[height - 1][width - 1] = Cell(CellType::C_Station);
}

int WarehouseMap::width() const {return width_;}
int WarehouseMap::height() const {return height_;}

void WarehouseMap::validatePosition(const Coordinate& pos) const {
    if (!isValid(pos)) {
        throw Positionerror("Position is out of grid");
    }
}

void WarehouseMap::validateCellForPlacement(const Coordinate& pos, const Cell& cell) const {
    if (cell.isShelf() && !cell.isPassable()) {
        throw Placementerror("Shelf must be on a passable cell type");
    }
}

const Cell& WarehouseMap::cellAt(const Coordinate& pos) const {
    validatePosition(pos);
    return grid_[pos.y()][pos.x()];
}

Cell& WarehouseMap::cellAt(const Coordinate& pos) {
    validatePosition(pos);
    return grid_[pos.y()][pos.x()];
}

void WarehouseMap::setCell(const Coordinate& pos, const Cell& cell) {
    validatePosition(pos);
    grid_[pos.y()][pos.x()] = cell;
}

bool WarehouseMap::isValid(const Coordinate& pos) const {
    return pos.x() >= 0 && pos.x() < width_ && pos.y() >= 0 && pos.y() < height_;
}

bool WarehouseMap::isPassable(const Coordinate& pos) const {
    return isValid(pos) && cellAt(pos).isPassable();
}

Coordinate WarehouseMap::pickupPosition() const {return pickupPos_;}

void WarehouseMap::setPickupPosition(const Coordinate& pos) {
    validatePosition(pos);
    grid_[pickupPos_.y()][pickupPos_.x()] = Cell(CellType::Empty);
    pickupPos_ = pos;
    grid_[pos.y()][pos.x()] = Cell(CellType::Pickup);
}

Coordinate WarehouseMap::chargingPosition() const {return chargingPos_;}

void WarehouseMap::setChargingPosition(const Coordinate& pos) {
    validatePosition(pos);
    grid_[chargingPos_.y()][chargingPos_.x()] = Cell(CellType::Empty);
    chargingPos_ = pos;
    grid_[pos.y()][pos.x()] = Cell(CellType::C_Station);
}

}