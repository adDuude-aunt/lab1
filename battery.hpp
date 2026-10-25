#pragma once
#include "domainerror.hpp"

namespace warehouse {

struct energyRange : DomainError {
    using DomainError::DomainError;
};

struct energyCost : DomainError {
    using DomainError::DomainError;
};

struct LackEnergy : DomainError {
    using DomainError::DomainError;
};

class Battery {
    private:
    int level_;
    int maxLevel_;
    public:
    Battery(int level, int m_level);

    int level() const;
    int maxLevel() const;
    bool isFull() const;
    bool hasEnough(int cost) const;

    void spend(int cost);
    void charge(int amount);

    bool operator==(const Battery& other) const;
};

}