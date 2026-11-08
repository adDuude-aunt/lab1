#include "dispatcher.hpp"
#include <algorithm>
#include <limits>

namespace warehouse {

static int calculateRequiredEnergy( const Coordinate& robotPos, const Coordinate& shelfPos,
    const Coordinate& pickupPos, int quantity, const WarehouseMap& map,
    const std::vector<Coordinate>& obstacles) {

    std::vector<Coordinate> pathToShelf;
    Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    for (auto d : dirs) {
        Coordinate neighbor = shelfPos.step(d);
        if (map.isValid(neighbor) && map.isPassable(neighbor)) {
            auto p = Pathfinding::findPath(robotPos, neighbor, map, obstacles);
            if (!p.empty() && (pathToShelf.empty() || p.size() < pathToShelf.size())) {
                pathToShelf = p;
            }
        }
    }
    if (pathToShelf.empty()) return -1; 
    
    int energyToShelf = static_cast<int>(pathToShelf.size()) - 1;
    
    Coordinate shelfNeighbor = pathToShelf.back();
    auto pathToPickup = Pathfinding::findPath(shelfNeighbor, pickupPos, map, obstacles);
    if (pathToPickup.empty()) return -1;
    
    int energyToPickup = (static_cast<int>(pathToPickup.size()) - 1) * (1 + quantity);
    
    int loadEnergy = quantity;
    int unloadEnergy = quantity;
    
    return energyToShelf + loadEnergy + energyToPickup + unloadEnergy + 1;
}



    void Dispatcher::assignOrders(std::vector<Order>& orders, std::vector<Robot>& robots, 
                              const WarehouseMap& map, EventJournal& journal, int tick) {
    
    auto findPathToShelf = [&](const Coordinate& start, const Coordinate& shelfPos, const std::vector<Coordinate>& obs) {
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
    std::vector<Order*> pendingOrders;
    for (auto& o : orders) {
        if (o.status() == OrderStatus::Pending) pendingOrders.push_back(&o);
    }
    std::sort(pendingOrders.begin(), pendingOrders.end(), [](Order* a, Order* b) {
        return a->quantity() > b->quantity();
    });

    for (auto* orderPtr : pendingOrders) {
        auto& order = *orderPtr;
        Coordinate shelfPos = order.shelfPosition();
        const Cell& shelf = map.cellAt(shelfPos);
        
        if (!shelf.isShelf() || shelf.stock() < order.quantity()) {
            order.LowStock();
            journal.log(tick, "Order", "Insufficient stock for order " + std::to_string(order.id()));
            continue;
        }

        int bestRobotId = -1;
        int bestDist = std::numeric_limits<int>::max();
        bool bestViaCharge = false;
        
        for (const auto& robot : robots) {  
            if (tasks_.count(robot.id()) || robot.isCharging()) continue;
            if (!robot.Carry(order.quantity())) continue;
            
            std::vector<Coordinate> obstacles;
            for (const auto& r : robots) if (r.id() != robot.id()) obstacles.push_back(r.position());
            
            auto pathDirect = findPathToShelf(robot.position(), order.shelfPosition(), obstacles);
            int requiredFromCurrent = -1;
            if (!pathDirect.empty()) {
                requiredFromCurrent = calculateRequiredEnergy(
                    robot.position(), order.shelfPosition(),
                    map.pickupPosition(), order.quantity(), map, obstacles);
            }
            bool canDirect = (requiredFromCurrent >= 0 && robot.battery().level() >= requiredFromCurrent);

            int effectiveDist = -1;
            bool viaCharge = false;

            if (canDirect) {
                effectiveDist = static_cast<int>(pathDirect.size()) - 1;
            } else {
                auto pathToCharge = Pathfinding::findPath(robot.position(), map.chargingPosition(), map, obstacles);
                if (!pathToCharge.empty()) {
                    int stepsToCharge = static_cast<int>(pathToCharge.size()) - 1;
                    int energyToReachCharge = stepsToCharge * robot.moveCost();
                    if (robot.battery().level() >= energyToReachCharge) {
                        int requiredFromCharge = calculateRequiredEnergy(
                            map.chargingPosition(), order.shelfPosition(),
                            map.pickupPosition(), order.quantity(), map, obstacles);
                        if (requiredFromCharge >= 0 && robot.battery().maxLevel() >= requiredFromCharge) {
                            effectiveDist = 1000 + stepsToCharge;
                            viaCharge = true;
                        }
                    }
                }
            }

            if (effectiveDist < 0) {
                journal.log(tick, "Charge",
                    "Robot " + std::to_string(robot.id()) +
                    " cannot complete order " + std::to_string(order.id()) +
                    " (direct needs " + std::to_string(requiredFromCurrent) +
                    ", has " + std::to_string(robot.battery().level()) +
                    ", max " + std::to_string(robot.battery().maxLevel()) + ")");
                continue;
            }

            if (effectiveDist < bestDist) {
                bestDist = effectiveDist;
                bestRobotId = robot.id();
                bestViaCharge = viaCharge;
            }
        }

        if (bestRobotId >= 0) {
            order.AttachedRobot(bestRobotId);
            order.InProgress();
            
            RobotTask task;
            task.robotId = bestRobotId;
            task.orderId = order.id();
            task.shelfPos = shelfPos;
            task.phase = RobotTask::Phase::ToShelf;
            task.resumePhase = RobotTask::Phase::Idle;
            task.pathIndex = (task.path.size() > 1) ? 1 : 0;
            task.loadAmount = order.quantity();
            
            std::vector<Coordinate> obstacles;
            for (const auto& r : robots) if (r.id() != bestRobotId) obstacles.push_back(r.position());
            
            Robot* rp = nullptr;
            for(auto& r: robots) if(r.id() == bestRobotId) rp = &r;
            
            task.path = findPathToShelf(rp->position(), shelfPos, obstacles);
            task.pathIndex = (task.path.size() > 1) ? 1 : 0;
            task.loadAmount = order.quantity();
            tasks_[bestRobotId] = task;
            journal.log(tick, "Assign", "Order " + std::to_string(order.id()) + " -> Robot " + std::to_string(bestRobotId));
        } 
    }
    for (auto& robot : robots) {
        if (tasks_.count(robot.id()) || robot.isCharging()) continue;
        if (robot.battery().level() <= chargeThreshold_) {
            std::vector<Coordinate> obstacles;
            for (const auto& r : robots) if (r.id() != robot.id()) obstacles.push_back(r.position());
            auto path = Pathfinding::findPath(robot.position(), map.chargingPosition(), map, obstacles);
            if (!path.empty()) {
                RobotTask task;
                task.robotId = robot.id();
                task.orderId = -1;
                task.phase = RobotTask::Phase::ToCharge;
                task.resumePhase = RobotTask::Phase::Idle;
                task.path = path;
                task.pathIndex = (task.path.size() > 1) ? 1 : 0;
                task.loadAmount = 0;
                tasks_[robot.id()] = task;
                journal.log(tick, "Charge", "Robot " + std::to_string(robot.id()) + " sent to charge (not enough energy for orders)");
            }
        }
    }
    
}
void Dispatcher::executeTick(std::vector<Robot>& robots, std::vector<Order>& orders,
                             WarehouseMap& map, EventJournal& journal, int tick) {
    for (auto& robot : robots) {
        auto it = tasks_.find(robot.id());
        if (it == tasks_.end()) continue;
        if (robot.isCharging()) continue;

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

  //      auto findPathToShelfLocal = [&](const Coordinate& start, const Coordinate& shelfPos, const std::vector<Coordinate>& obs) {
  //          std::vector<Coordinate> best_path;
  //          Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
  //          for (auto d : dirs) {
  //              Coordinate neighbor = shelfPos.step(d);
  //              if (map.isValid(neighbor) && map.isPassable(neighbor)) {
 //                   auto p = Pathfinding::findPath(start, neighbor, map, obs);
  //                  if (!p.empty() && (best_path.empty() || p.size() < best_path.size())) {
  //                      best_path = p;
  //                  }
  //              }
  //          }
 //           return best_path;
 //       };

        switch (task.phase) {
            case RobotTask::Phase::ToShelf:
                 if (task.pathIndex >= (int)task.path.size()) {
                    task.phase = RobotTask::Phase::Loading;
                } else {
                    if (!robot.able2move()) {
                        task.resumePhase = task.phase; // ToShelf
                        task.phase = RobotTask::Phase::ToCharge;
                        std::vector<Coordinate> obstacles;
                        for (const auto& r : robots) if (r.id() != robot.id()) obstacles.push_back(r.position());
                        task.path = Pathfinding::findPath(robot.position(), map.chargingPosition(), map, obstacles);
                        task.pathIndex = 0;
                        journal.log(tick, "Charge", "Robot " + std::to_string(robot.id()) + " going to charge (out of battery during ToShelf)");
                    } else {
                        tryMove();
                    }
                }
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
                    task.pathIndex = (task.path.size() > 1) ? 1 : 0; 
                } else if (amt == 0) {
                    for (auto& o : orders) {
                        if (o.id() == task.orderId) {
                             o.Cancelled();
                            journal.log(tick, "Order", "Order " + std::to_string(o.id()) + " cancelled: no stock");
                            break;
                        }
                    }
                    task.phase = RobotTask::Phase::Idle;
                } else if (!robot.battery().hasEnough(amt)) {
                    task.resumePhase = task.phase;
                    task.phase = RobotTask::Phase::ToCharge;
                    std::vector<Coordinate> obstacles;
                    task.path = Pathfinding::findPath(robot.position(), map.chargingPosition(), map, obstacles);
                    task.pathIndex = 0;
                    journal.log(tick, "Charge", "Robot " + std::to_string(robot.id()) + " going to charge (not enough energy to load)");
                }
                break;
            }

            case RobotTask::Phase::ToPickup: {
                if (task.path.empty()) {
                    task.phase = RobotTask::Phase::Unloading;
                    break;
                }

                if (task.pathIndex >= (int)task.path.size()) {
                    task.phase = RobotTask::Phase::Unloading;
                } else {
                    int distanceToPickup = static_cast<int>(task.path.size()) - task.pathIndex;
                    int unloadCost = robot.deliver();
                    int moveCost = distanceToPickup * robot.moveCost();
                    int totalNeeded = moveCost + unloadCost;

                    if (robot.battery().level() < totalNeeded || !robot.able2move()) {
                        task.resumePhase = task.phase; 
                        task.phase = RobotTask::Phase::ToCharge;
                        std::vector<Coordinate> obstacles;
                        task.path = Pathfinding::findPath(
                            robot.position(), map.chargingPosition(), map, obstacles);
                        task.pathIndex = (task.path.size() > 1) ? 1 : 0;
                        journal.log(tick, "Charge",
                            "Robot " + std::to_string(robot.id()) +
                            " going to charge (not enough energy to reach pickup)");
                    } else {
                        tryMove();
                    }
                }
                break;
            }

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
                    
                    task.phase = RobotTask::Phase::leaving;
                    } else {
                        task.resumePhase = task.phase;
                        task.phase = RobotTask::Phase::ToCharge;
                        std::vector<Coordinate> obstacles;
                        for (const auto& r : robots) if (r.id() != robot.id()) obstacles.push_back(r.position());
                        task.path = Pathfinding::findPath(
                            robot.position(), map.chargingPosition(), map, obstacles);
                        task.pathIndex = (task.path.size() > 1) ? 1 : 0;
                        journal.log(tick, "Charge",
                            "Robot " + std::to_string(robot.id()) +
                            " going to charge (not enough energy to unload)");
                    }
                } else if (robot.position() == map.pickupPosition() && robot.deliver() == 0) {
                    for (auto& o : orders) {
                        if (o.id() == task.orderId) {
                            o.Completed();
                            journal.log(tick, "Order", "Order " + std::to_string(o.id()) + " completed (empty cargo)");
                            break;
                        }
                    }
                    task.phase = RobotTask::Phase::Idle;
                } else {
                    task.phase = RobotTask::Phase::ToPickup;
                    std::vector<Coordinate> obstacles;
                    task.path = Pathfinding::findPath(robot.position(), map.pickupPosition(), map, obstacles);
                    task.pathIndex = (task.path.size() > 1) ? 1 : 0;
                }
                break;

            case RobotTask::Phase::leaving: {
                if (robot.position() != map.pickupPosition()) {
                    task.phase = RobotTask::Phase::Idle;
                    break;
                }

                Coordinate target = robot.position();
                bool found = false;
                Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
                for (auto d : dirs) {
                    Coordinate neighbor = robot.position().step(d);
                    if (map.isValid(neighbor) && map.isPassable(neighbor)) {
                        bool occupied = false;
                        for (const auto& r : robots) {
                            if (r.id() != robot.id() && r.position() == neighbor) {
                                occupied = true;
                                break;
                            }
                        }
                        if (!occupied) {
                            target = neighbor;
                            found = true;
                            break;
                        }
                    }
                }

                if (found) {
                    if (MovementRules::canMove(robot, target, map, robots)) {
                        robot.moveTo(target);
                        journal.log(tick, "Move",
                            "Robot " + std::to_string(robot.id()) +
                            " left pickup to " + std::to_string(target.x()) + "," + std::to_string(target.y()));
                        task.phase = RobotTask::Phase::Idle;
                    }
                }
                break;
            }

            case RobotTask::Phase::ToCharge:
                if (!tryMove()) {
                    if (robot.position() == map.chargingPosition()) {
                        robot.setCharging(true);
                        journal.log(tick, "Charge",
                            "Robot " + std::to_string(robot.id()) + " started charging");
                    }
                    task.phase = RobotTask::Phase::Idle;
                } else {
                    if (!robot.able2move()) {
                    } else {
                        tryMove();
                    }
                }
                break;

            case RobotTask::Phase::Idle:
                break;
        }
    }

     for (auto& robot : robots) {
        if (robot.isCharging() && robot.position() == map.chargingPosition()) {
            if (!robot.battery().isFull()) {
                int charge_amount = chargeAmount_;
                if (robot.battery().level() + charge_amount > robot.battery().maxLevel()) {
                    charge_amount = robot.battery().maxLevel() - robot.battery().level();
                }
                robot.charge(charge_amount);
                journal.log(tick, "Charge", 
                    "Robot " + std::to_string(robot.id()) + " charged +" + std::to_string(charge_amount));
                } else {
                robot.setCharging(false);
                journal.log(tick, "Charge", 
                    "Robot " + std::to_string(robot.id()) + " fully charged");

                auto it = tasks_.find(robot.id());
                if (it != tasks_.end()) {
                    RobotTask& task = it->second;
                    if (task.resumePhase != RobotTask::Phase::Idle && task.orderId != -1) {
                        task.phase = task.resumePhase;
                        task.resumePhase = RobotTask::Phase::Idle;
                        std::vector<Coordinate> obstacles;
                        for (const auto& r : robots) if (r.id() != robot.id()) obstacles.push_back(r.position());

                        if (task.phase == RobotTask::Phase::ToShelf || task.phase == RobotTask::Phase::Loading) {
                            auto findPathToShelfLocal = [&](const Coordinate& start, const Coordinate& shelfPos, const std::vector<Coordinate>& obs) {
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
                            task.path = findPathToShelfLocal(robot.position(), task.shelfPos, obstacles);
                            task.pathIndex = 0;
                        } else if (task.phase == RobotTask::Phase::ToPickup || task.phase == RobotTask::Phase::Unloading) {
                            task.path = Pathfinding::findPath(robot.position(), map.pickupPosition(), map, obstacles);
                            task.pathIndex = 0;
                        } else {
                            task.phase = RobotTask::Phase::Idle;
                        }
                        journal.log(tick, "Charge", "Robot " + std::to_string(robot.id()) + " resuming task after charging");
                    } else {
                        task.phase = RobotTask::Phase::Idle;
                    }
                }
            }
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
}

void Dispatcher::assignTask(int robotId, int orderId, Coordinate shelfPos, int quantity, 
                                  const WarehouseMap& map, const std::vector<Robot>& robots, 
                                  EventJournal& journal, int tick) {
    RobotTask task;
    task.robotId = robotId;
    task.orderId = orderId;
    task.shelfPos = shelfPos;
    task.phase = RobotTask::Phase::ToShelf;
    task.resumePhase = RobotTask::Phase::Idle;
    task.loadAmount = quantity;

    std::vector<Coordinate> obstacles;
    Coordinate robotPos;
    for (const auto& r : robots) {
        if (r.id() == robotId) robotPos = r.position();
    }

    std::vector<Coordinate> best_path;
    Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    for (auto d : dirs) {
        Coordinate neighbor = shelfPos.step(d);
        if (map.isValid(neighbor) && map.isPassable(neighbor)) {
            auto p = Pathfinding::findPath(robotPos, neighbor, map, obstacles);
            if (!p.empty() && (best_path.empty() || p.size() < best_path.size())) {
                best_path = p;
            }
        }
    }
    
    task.path = best_path;
    task.pathIndex = (best_path.size() > 1) ? 1 : 0;
    tasks_[robotId] = task;
    journal.log(tick, "Assign", "Order " + std::to_string(orderId) + " -> Robot " + std::to_string(robotId) + " (manual)");
}

}



