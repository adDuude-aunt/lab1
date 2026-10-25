#pragma once
#include "domainerror.hpp"

namespace warehouse {

enum class CellType {
    Empty, Shelf, Pickup, C_Station, Wall
};

struct CellState : DomainError {
    using DomainError::DomainError;
};

class Cell {
    private:
        CellType type_;
        int stock_;

        void validate() const {
            if (stock_ < 0) {
                throw CellState("Stock can`t be negative");
            }
            if (type_ != CellType::Shelf && stock_ > 0) {
                throw CellState("Only shelves can contain stock");
            }
        }

    public:
        explicit Cell(CellType type = CellType::Empty, int stock = 0);

        CellType type() const;
        int stock() const;

        void addStock(int amount);
        void removeStock(int amount);

        bool isPassable() const;
        bool isShelf() const;
        bool isPickup() const;
        bool isChargingStation() const;

        bool operator==(const Cell& other) const;
};

}