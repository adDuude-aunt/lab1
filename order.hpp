#pragma once
#include "coordinate.hpp"
#include "domainerror.hpp"

namespace warehouse {

enum class OrderStatus {
    Pending, Setted, InProgress, Completed, Cancelled, LowStock
};

struct StateError : DomainError {
    using DomainError::DomainError;
};

struct QuantityError : DomainError {
    using DomainError::DomainError;
};

class Order {
private:
    int id_;
    Coordinate shelfPos_;
    int quantity_;
    OrderStatus status_;
    int botId_;

    void requireStatus(OrderStatus required) const;

public:
    Order(int id, Coordinate shelfPos, int quantity);

    int id() const;
    Coordinate shelfPosition() const;
    int quantity() const;
    OrderStatus status() const;
    int botId() const;

    void AttachedRobot(int botId);
    void InProgress();
    void Completed();
    void Cancelled();
    void LowStock();

    bool isPending() const;
    bool isSetted() const;
    bool isCompleted() const;
    bool isCancelled() const;

    bool operator==(const Order& other) const;
};

}