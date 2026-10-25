#include "cell.hpp"

namespace warehouse {

    Cell::Cell(CellType type, int stock) : type_(type), stock_(stock) {
        validate();
    }

    CellType Cell::type() const { return type_; }
    int Cell::stock() const { return stock_; }


    void Cell::addStock(int amount) {
        if (amount < 0)
            throw CellState("Cannot add negative amount");
        stock_ += amount;
        validate();
    }

    void Cell::removeStock(int amount) {
        if (amount < 0)
            throw CellState("Cannot remove negative amount");
        if (amount > stock_)
            throw CellState("Not enough stock to remove");
        stock_ -= amount;
    }

    bool Cell::isPassable() const {
        return type_ != CellType::Wall && type_ != CellType::Shelf;
    }

    bool Cell::isShelf() const {return type_ == CellType::Shelf;}
    bool Cell::isPickup() const {return type_ == CellType::Pickup;}
    bool Cell::isChargingStation() const {return type_ == CellType::C_Station;}

    bool Cell::operator==(const Cell& other) const {
        return type_ == other.type_ && stock_ == other.stock_;
    }

}