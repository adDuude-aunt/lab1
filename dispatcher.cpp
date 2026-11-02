#include "dispatcher.hpp"
#include <algorithm>
#include <limits>

namespace warehouse {

void Dispatcher::assignOrders(std::vector<Order>& orders, std::vector<Robot>& robots,
                              const WarehouseMap& map, EventJournal& journal, int tick) {
    auto findPathToShelf = [&](const Coordinate& start, const Coordinate& shelfPos,
                               const std::vector<Coordinate>& obs) {
        std::vector<Coordinate> best_path;
        Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
        for (auto d : dirs) {
            Coordinate neighbor = shelfPos.step(d);
            if (map.isValid(neighbor) && map.isPassable(neighbor)) {
                auto p = Pathfinding::findPath(start, neighbor, map, obs);
                if (!p.empty() && (best_path.empty() || p.size() < best_path.size())) {
                    best_path = p;
                }
            }
        }
        return best_path;
    };

    for (auto& order : orders) {
        if (order.status() != OrderStatus::Pending) continue;

        const Cell& shelf = map.cellAt(order.shelfPosition());
        if (!shelf.isShelf() || shelf.stock() < order.quantity()) {
            order.LowStock();
            journal.log(tick, "Order", "Insufficient stock for order " + std::to_string(order.id()));
            continue;
        }

        int bestRobotId = -1;
        int bestDist = std::numeric_limits<int>::max();

        for (const auto& robot : robots) {
            if (tasks_.count(robot.id()) || robot.isCharging()) continue;
            if (robot.battery().level() <= chargeThreshold_) continue;
            if (!robot.Carry(order.quantity())) continue;

            std::vector<Coordinate> obstacles;
            for (const auto& r : robots)
                if (r.id() != robot.id()) obstacles.push_back(r.position());

            auto path = findPathToShelf(robot.position(), order.shelfPosition(), obstacles);
            int dist = path.empty() ? -1 : static_cast<int>(path.size()) - 1;

            if (dist >= 0 && dist < bestDist) {
                bestDist = dist;
                bestRobotId = robot.id();
            }
        }

        if (bestRobotId >= 0) {
            order.AttachedRobot(bestRobotId);
            order.InProgress();

            RobotTask task;
            task.robotId = bestRobotId;
            task.orderId = order.id();
            task.shelfPos = order.shelfPosition();
            task.phase = RobotTask::Phase::ToShelf;
            task.pathIndex = 0;
            task.loadAmount = order.quantity();

            std::vector<Coordinate> obstacles;
            for (const auto& r : robots)
                if (r.id() != bestRobotId) obstacles.push_back(r.position());

            Robot* rp = nullptr;
            for (auto& r : robots)
                if (r.id() == bestRobotId) rp = &r;

            task.path = findPathToShelf(rp->position(), order.shelfPosition(), obstacles);
            tasks_[bestRobotId] = task;

            journal.log(tick, "Assign",
                "Order " + std::to_string(order.id()) +
                " -> Robot " + std::to_string(bestRobotId));
        }
    }
}

void Dispatcher::executeTick(std::vector<Robot>& robots, std::vector<Order>& orders,
                             WarehouseMap& map, EventJournal& journal, int tick) {
    for (auto& robot : robots) {
        auto it = tasks_.find(robot.id());
        if (it == tasks_.end()) continue;

        RobotTask& task = it->second;

        auto tryMove = [&]() -> bool {
            if (task.pathIndex >= (int)task.path.size()) return false;
            Coordinate next = task.path[task.pathIndex];
            if (MovementRules::canMove(robot, next, map, robots)) {
                robot.moveTo(next);
                task.pathIndex++;
                journal.log(tick, "Move",
                    "Robot " + std::to_string(robot.id()) +
                    " moved to " + std::to_string(next.x()) + "," + std::to_string(next.y()));
                return true;
            }
            return false;
        };

        switch (task.phase) {
            case RobotTask::Phase::ToShelf:
                if (!tryMove()) task.phase = RobotTask::Phase::Loading;
                break;

            case RobotTask::Phase::Loading: {
                Cell& shelf = map.cellAt(task.shelfPos);
                int amt = std::min({task.loadAmount, shelf.stock(), robot.freeCapacity()});

                if (amt > 0 && robot.battery().hasEnough(amt)) {
                    shelf.removeStock(amt);
                    robot.load(amt);
                    robot.spendEnergy(amt);
                    journal.log(tick, "Load",
                        "Robot " + std::to_string(robot.id()) + " loaded " + std::to_string(amt));

                    task.phase = RobotTask::Phase::ToPickup;
                    std::vector<Coordinate> obstacles;
                    for (const auto& r : robots)
                        if (r.id() != robot.id()) obstacles.push_back(r.position());
                    task.path = Pathfinding::findPath(robot.position(), map.pickupPosition(), map, obstacles);
                    task.pathIndex = 0;
                } else if (robot.battery().level() <= chargeThreshold_) {
                    task.phase = RobotTask::Phase::ToCharge;
                    std::vector<Coordinate> obstacles;
                    for (const auto& r : robots)
                        if (r.id() != robot.id()) obstacles.push_back(r.position());
                    task.path = Pathfinding::findPath(robot.position(), map.chargingPosition(), map, obstacles);
                    task.pathIndex = 0;
                }
                break;
            }

            case RobotTask::Phase::ToPickup:
                if (!tryMove()) task.phase = RobotTask::Phase::Unloading;
                break;

            case RobotTask::Phase::Unloading:
                if (robot.position() == map.pickupPosition() && robot.deliver() > 0) {
                    int amt = robot.deliver();
                    if (robot.battery().hasEnough(amt)) {
                        robot.unload(amt);
                        robot.spendEnergy(amt);
                        journal.log(tick, "Unload",
                            "Robot " + std::to_string(robot.id()) + " unloaded " + std::to_string(amt));

                        for (auto& o : orders) {
                            if (o.id() == task.orderId) {
                                o.Completed();
                                journal.log(tick, "Order",
                                    "Order " + std::to_string(o.id()) + " completed");
                                break;
                            }
                        }
                    }
                    task.phase = RobotTask::Phase::Idle;
                } else {
                    task.phase = RobotTask::Phase::Idle;
                }
                break;

            case RobotTask::Phase::ToCharge:
                if (!tryMove()) {
                    if (robot.position() == map.chargingPosition()) {
                        robot.setCharging(true);
                        journal.log(tick, "Charge",
                            "Robot " + std::to_string(robot.id()) + " started charging");
                    }
                    task.phase = RobotTask::Phase::Idle;
                }
                break;

            case RobotTask::Phase::Idle:
                break;
        }
    }

    std::vector<int> done;
    for (auto& [id, task] : tasks_)
        if (task.phase == RobotTask::Phase::Idle) done.push_back(id);
    for (int id : done) tasks_.erase(id);
}

void Dispatcher::sendToCharge(std::vector<Robot>& robots, const WarehouseMap& map,
                              EventJournal& journal, int tick) {
    for (auto& robot : robots) {
        if (tasks_.count(robot.id()) || robot.isCharging()) continue;
        if (robot.battery().level() <= chargeThreshold_) {
            std::vector<Coordinate> obstacles;
            for (const auto& r : robots)
                if (r.id() != robot.id()) obstacles.push_back(r.position());

            auto path = Pathfinding::findPath(robot.position(), map.chargingPosition(), map, obstacles);
            if (!path.empty()) {
                RobotTask task;
                task.robotId = robot.id();
                task.orderId = -1;
                task.phase = RobotTask::Phase::ToCharge;
                task.path = path;
                task.pathIndex = 0;
                task.loadAmount = 0;
                tasks_[robot.id()] = task;
                journal.log(tick, "Charge",
                    "Robot " + std::to_string(robot.id()) + " sent to charge");
            }
        }
    }

    for (auto& robot : robots) {
        if (robot.isCharging() && robot.position() == map.chargingPosition()) {
            robot.charge(chargeAmount_);
            if (robot.battery().isFull()) {
                robot.setCharging(false);
                journal.log(tick, "Charge",
                    "Robot " + std::to_string(robot.id()) + " fully charged");
            }
        }
    }
}

}