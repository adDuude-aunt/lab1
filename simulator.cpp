#include "simulator.hpp"
#include "domainerror.hpp"

namespace warehouse {

Simulator::Simulator(WarehouseMap map, std::vector<Robot> robots)
    : map_(std::move(map)), robots_(std::move(robots)), currentTick_(0), nextOrderId_(1) {}

void Simulator::tick() {
    currentTick_++;
    dispatcher_.assignOrders(orders_, robots_, map_, journal_, currentTick_);
    dispatcher_.sendToCharge(robots_, map_, journal_, currentTick_);
    dispatcher_.executeTick(robots_, orders_, map_, journal_, currentTick_);
    collectStats();
}

int Simulator::currentTick() const { return currentTick_; }

void Simulator::createOrder(Coordinate shelfPos, int quantity) {
    Order order(nextOrderId_++, shelfPos, quantity);
    orders_.push_back(order);
    journal_.log(currentTick_, "Order", "Created order " + std::to_string(order.id()));
}

void Simulator::assignOrderToRobot(int orderId, int robotId) {
    Order* order = findOrder(orderId);
    if (!order) throw DomainError("Order not found");
    if (order->status() != OrderStatus::Pending) throw DomainError("Order is not pending");
    Robot* robot = findRobot(robotId);
    if (!robot) throw DomainError("Robot not found");
    
    order->AttachedRobot(robotId);
    order->InProgress();
    journal_.log(currentTick_, "Assign", "Order " + std::to_string(orderId) + " -> Robot " + std::to_string(robotId));
}

void Simulator::moveRobot(int robotId, Direction dir) {
    Robot* robot = findRobot(robotId);
    if (!robot) throw DomainError("Robot not found");
    Coordinate target = robot->position().step(dir);
    if (!MovementRules::canMove(*robot, target, map_, robots_)) throw DomainError("Cannot move");
    robot->moveTo(target);
    journal_.log(currentTick_, "Move", "Robot " + std::to_string(robotId) + " moved manually");
}

void Simulator::loadCargo(int robotId, int amount) {
    Robot* robot = findRobot(robotId);
    if (!robot) throw DomainError("Robot not found");
    Cell& cell = map_.cellAt(robot->position());
    if (!MovementRules::canLoad(*robot, cell, amount)) throw DomainError("Cannot load");
    cell.removeStock(amount);
    robot->load(amount);
    robot->spendEnergy(amount);
    journal_.log(currentTick_, "Load", "Robot " + std::to_string(robotId) + " loaded " + std::to_string(amount));
}

void Simulator::unloadCargo(int robotId, int amount) {
    Robot* robot = findRobot(robotId);
    if (!robot) throw DomainError("Robot not found");
    Cell& cell = map_.cellAt(robot->position());
    if (!MovementRules::canUnload(*robot, cell, amount)) throw DomainError("Cannot unload");
    robot->unload(amount);
    robot->spendEnergy(amount);
    journal_.log(currentTick_, "Unload", "Robot " + std::to_string(robotId) + " unloaded " + std::to_string(amount));
}

void Simulator::sendRobotToCharge(int robotId) {
    Robot* robot = findRobot(robotId);
    if (!robot) throw DomainError("Robot not found");
    if (robot->position() != map_.chargingPosition()) throw DomainError("Not at charging station");
    robot->setCharging(true);
    journal_.log(currentTick_, "Charge", "Robot " + std::to_string(robotId) + " charging");
}

const WarehouseMap& Simulator::map() const { return map_; }
const std::vector<Robot>& Simulator::robots() const { return robots_; }
const std::vector<Order>& Simulator::orders() const { return orders_; }
const EventJournal& Simulator::journal() const { return journal_; }
const std::vector<TickStats>& Simulator::stats() const { return stats_; }

void Simulator::saveLog(const std::string& filename) const { journal_.save(filename); }
void Simulator::loadLog(const std::string& filename) { journal_.load(filename); }

Robot* Simulator::findRobot(int id) {
    for (auto& r : robots_) if (r.id() == id) return &r;
    return nullptr;
}
Order* Simulator::findOrder(int id) {
    for (auto& o : orders_) if (o.id() == id) return &o;
    return nullptr;
}

void Simulator::collectStats() {
    TickStats s;
    s.tick = currentTick_;
    s.pendingOrders = s.completedOrders = 0;
    for (const auto& o : orders_) {
        if (o.status() == OrderStatus::Pending || o.status() == OrderStatus::Setted || o.status() == OrderStatus::InProgress) s.pendingOrders++;
        if (o.status() == OrderStatus::Completed) s.completedOrders++;
    }
    for (const auto& r : robots_) s.robotCharges.push_back(r.battery().level());
    stats_.push_back(s);
}

}