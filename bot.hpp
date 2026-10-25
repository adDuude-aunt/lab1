#pragma once
#include "coordinate.hpp"
#include "battery.hpp"
#include "domainerror.hpp"

namespace warehouse {

    struct CapacityError : DomainError {
        using DomainError::DomainError;
    };

    struct DeliverError : DomainError {
        using DomainError::DomainError;
    };

    struct StateError : DomainError {
        using DomainError::DomainError;
    };

    class Robot {
    private:
        int id_;
        Coordinate position_;
        Battery battery_;
        int capacity_;
        int deliver_;
        bool isCharging_;

        void validateDeliver(int amount) const;
        void validateCapacity(int total) const;

    public:
        Robot(int id, Coordinate position, int maxBattery, int capacity);

        int id() const;

        Coordinate position() const;
        void moveTo(Coordinate newPos);

        const Battery& battery() const;
        void spendEnergy(int amount);
        void charge(int amount);
        bool isCharging() const;
        void setCharging(bool charging);


        int capacity() const;
        int deliver() const;
        int freeCapacity() const;
        void load(int amount);
        void unload(int amount);


        int moveCost() const;
        bool Carry(int amount) const;
        bool able2move() const;

        bool operator==(const Robot& other) const;
    };

} 