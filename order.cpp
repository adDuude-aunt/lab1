#include "order.hpp"

namespace warehouse {

    Order::Order(int id, Coordinate shelf, int quantity)
        : id_(id), shelf_(shelf), quantity_(quantity), status_(OrderStatus::Pending), RobotId_(-1) {
        if (id < 0) {
            throw StateError("Order ID can`t be negative");
        }
        if (quantity <= 0) {
            throw QuantityError("Order quantity must be positive");
        }
    }

    int Order::id() const {return id_;}
    int Order::RobotId() const {return RobotId_;}
    Coordinate Order::shelf() const {return shelf_;}
    int Order::quantity() const {return quantity_;}
    OrderStatus Order::status() const {return status_;}

    void Order::requireStatus(OrderStatus required) const {
        if (status_ != required) {
            throw StateError(
                "Order status is " + std::to_string(static_cast<int>(status_))
            );
        }
    }

    void Order::assignToRobot(int botId) {
        requireStatus(OrderStatus::Pending);
        if (robotId < 0) {
            throw StateError("Robot ID can`t be negative");
        }
        RobotId_ = botId;
        status_ = OrderStatus::Assigned;
    }

    void Order::InProgress() {
        requireStatus(OrderStatus::Assigned);
        status_ = OrderStatus::InProgress;
    }

    void Order::Completed() {
        requireStatus(OrderStatus::InProgress);
        status_ = OrderStatus::Completed;
    }

    void Order::Cancelled() {
        if (status_ == OrderStatus::Completed) {
            throw StateError("Can`t cancel completed order");
        }
        status_ = OrderStatus::Cancelled;
    }

    void Order::LowStock() {
        if (status_ != OrderStatus::Pending) {
            throw StateError("Can only mark low stock for pending orders");
        }
        status_ = OrderStatus::LowStock;
    }

    bool Order::isPending() const {return status_ == OrderStatus::Pending;}
    bool Order::isAssigned() const {return status_ == OrderStatus::Assigned;}
    bool Order::isCompleted() const {return status_ == OrderStatus::Completed;}
    bool Order::isCancelled() const {return status_ == OrderStatus::Cancelled;}

    bool Order::operator==(const Order& other) const {
        return id_ == other.id_;
    }

} 