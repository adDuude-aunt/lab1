#include "bot.hpp"

namespace warehouse {

    Robot::Robot(int id, Coordinate position, int maxBattery, int capacity)
        : id_(id), position_(position), battery_(maxBattery, maxBattery),
        capacity_(capacity), deliver_(0), isCharging_(false) {
        if (id < 0) {
            throw StateError("Robot ID can`t be negative");
        }
        if (capacity <= 0) {
            throw CapacityError("Robot capacity can`t be negative");
        }
    }

    int Robot::id() const { return id_; }

    Coordinate Robot::position() const { return position_; }

    void Robot::moveTo(Coordinate newPos) {
        if (isCharging_) {
            throw StateError("Can`t move when charging");
        }
        if (!able2move()) {
            throw StateError("Not enough energy to move");
        }
        position_ = newPos;
        battery_.spend(moveCost());
    }

    const Battery& Robot::battery() const { return battery_; }

    void Robot::spendEnergy(int amount) {
        battery_.spend(amount);
    }

    void Robot::charge(int amount) {
        if (!isCharging_) {
            throw StateError("Can only charge when charging flag is set");
        }
        battery_.charge(amount);
    }

    bool Robot::isCharging() const { return isCharging_; }

    void Robot::setCharging(bool charging) {
        isCharging_ = charging;
    }

    int Robot::capacity() const { return capacity_; }

    int Robot::deliver() const { return deliver_; }

    int Robot::freeCapacity() const { return capacity_ - deliver_; }

    void Robot::validateDeliver(int amount) const {
        if (amount < 0) {
            throw DeliverError("Amount can`t be negative");
        }
    }

    void Robot::validateCapacity(int total) const {
        if (total > capacity_) {
            throw CapacityError("Exceeds robot capacity");
        }
    }

    void Robot::load(int amount) {
        validateDeliver(amount);
        validateCapacity(deliver_ + amount);
        deliver_ += amount;
    }

    void Robot::unload(int amount) {
        validateDeliver(amount);
        if (amount > deliver_) {
            throw DeliverError("Not enough cargo to unload");
        }
        deliver_ -= amount;
    }

    int Robot::moveCost() const {
        return 1 + deliver_;
    }

    bool Robot::Carry(int amount) const {
        return deliver_ + amount <= capacity_;
    }

    bool Robot::able2move() const {
        return battery_.hasEnough(moveCost());
    }

    bool Robot::operator==(const Robot& other) const {
        return id_ == other.id_;
    }

} 