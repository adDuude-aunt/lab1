#include "battery.hpp"

namespace warehouse {

Battery::Battery(int level, int maxLevel) : level_(level), maxLevel_(maxLevel) {
    if (maxLevel_ < 1 || level_ < 0 || level_ > maxLevel_)
        throw energyRange("The battery charge cannot be that level");
}

int Battery::level() const {
    return level_;
}

int Battery::maxLevel() const {
    return maxLevel_;
}

bool Battery::isFull() const {
    return level_ == maxLevel_;
}

bool Battery::hasEnough(int cost) const {
    return cost <= level_;
}

void Battery::spend(int cost) {
    if (cost < 0)
        throw energyCost("energy cost can`t be negative");

    if (cost > level_)
        throw LackEnergy("battery has not enough charge");

    level_ -= cost;
}

void Battery::charge(int amount) {
    if (amount < 0)
        throw energyCost("charge amount can`t be negative");

    level_ += amount;

    if (level_ > maxLevel_)
        level_ = maxLevel_;
}

bool Battery::operator==(const Battery& other) const {
    return level_ == other.level_ && maxLevel_ == other.maxLevel_;
}

}