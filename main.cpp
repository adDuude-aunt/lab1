#include "map.hpp"
#include "bot.hpp"
#include "order.hpp"
#include "simulator.hpp"
#include "graphs.hpp"
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iomanip>

using namespace warehouse;

const int CHARGE_RATE = 5;

enum class Mode { Manual, Auto };

static WarehouseMap* g_map = nullptr;
static std::vector<Robot>* g_robots = nullptr;
static Simulator* g_sim = nullptr;
static Mode g_mode = Mode::Manual;

static std::string format_cell(const std::string& raw) {
    if (raw.size() >= 3) return raw.substr(0, 3);
    std::string padded = raw;
    while (padded.size() < 3) padded += ' ';
    return padded;
}

static std::string center_number(int n, int width = 5) {
    std::string s = std::to_string(n);
    if (static_cast<int>(s.size()) >= width) return s.substr(0, width);
    int padding = width - static_cast<int>(s.size());
    int left = padding / 2;
    int right = padding - left;
    return std::string(left, ' ') + s + std::string(right, ' ');
}

void print_map() {
    std::cout << "\n";
    std::cout << "    ";
    for (int x = 0; x < g_map->width(); ++x) {
        std::cout << center_number(x, 5);
    }
    std::cout << "\n";
    std::cout << "     ";
    for (int x = 0; x < g_map->width(); ++x) std::cout << "-----";
    std::cout << "\n";

    for (int y = 0; y < g_map->height(); ++y) {
        std::cout << std::setw(2) << y << "  ";
        for (int x = 0; x < g_map->width(); ++x) {
            Coordinate pos(x, y);
            bool has_robot = false;
            for (const auto& r : *g_robots) {
                if (r.position() == pos) {
                    std::string raw = "R" + std::to_string(r.id());
                    std::cout << "[" << format_cell(raw) << "]";
                    has_robot = true;
                    break;
                }
            }
            if (has_robot) continue;

            CellType type = g_map->cellAt(pos).type();
            std::string raw;
            switch (type) {
                case CellType::Empty:           raw = " . "; break;
                case CellType::Shelf: {
                    int stock = g_map->cellAt(pos).stock();
                    if (stock >= 100) raw = "S99";
                    else if (stock >= 10) raw = "S" + std::to_string(stock);
                    else raw = "S" + std::to_string(stock) + " ";
                    break;
                }
                case CellType::Pickup:raw = " P "; break;
                case CellType::C_Station:raw = " C "; break;
                case CellType::Wall:raw = " # "; break;
            }
            std::cout << "[" << raw << "]";
        }
        std::cout << "\n";
    }
    std::cout << "     ";
    for (int x = 0; x < g_map->width(); ++x) std::cout << "-----";
    std::cout << "\n";
    std::cout << "R-робот, S-стеллаж(товар), P-выдача, C-зарядка, #-стена\n";
}

void print_robots() {
    std::cout << "\n--- Статус роботов ---\n";
    for (const auto& r : *g_robots) {
        std::cout << "Робот " << r.id()
                  << " @ (" << r.position().x() << "," << r.position().y() << ")"
                  << " | Батарея: " << r.battery().level() << "/" << r.battery().maxLevel()
                  << " | Груз: " << r.deliver() << "/" << r.capacity()
                  << (r.isCharging() ? " [ЗАРЯДКА]" : "")
                  << (r.battery().isFull() ? " [ПОЛНЫЙ]" : "")
                  << "\n";
    }
    std::cout << "----------------------\n";
}

void print_orders() {
    const auto& orders = g_sim->orders();
    std::cout << "\n--- Заявки ---\n";
    if (orders.empty()) {
        std::cout << "(нет заявок)\n";
    } else {
        for (const auto& o : orders) {
            std::cout << "Заявка #" << o.id()
                      << " | Стеллаж: (" << o.shelfPosition().x() << "," << o.shelfPosition().y() << ")"
                      << " | Кол-во: " << o.quantity()
                      << " | Статус: ";
            switch (o.status()) {
                case OrderStatus::Pending:     std::cout << "Ожидает назначения"; break;
                case OrderStatus::Setted:      std::cout << "Назначен"; break;
                case OrderStatus::InProgress:  std::cout << "Выполняется"; break;
                case OrderStatus::Completed:   std::cout << "Выполнен"; break;
                case OrderStatus::Cancelled:   std::cout << "Отменён"; break;
                case OrderStatus::LowStock:    std::cout << "Мало товара"; break;
            }
            if (o.botId() >= 0) std::cout << " | Робот: " << o.botId();
            std::cout << "\n";
        }
    }
    std::cout << "----------------\n";
}

std::vector<AdjacentShelf> find_adjacent_shelves(Robot& robot) {
    std::vector<AdjacentShelf> result;
    Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    for (auto d : dirs) {
        Coordinate neighbor = robot.position().step(d);
        if (g_map->isValid(neighbor)) {
            Cell& cell = g_map->cellAt(neighbor);
            if (cell.isShelf() && cell.stock() > 0) {
                result.push_back({d, &cell, neighbor});
            }
        }
    }
    return result;
}

void charge_tick() {
    std::cout << "\n=== Зарядка ===\n";
    for (auto& robot : *g_robots) {
        if (robot.isCharging() && g_map->cellAt(robot.position()).isChargingStation()) {
            if (!robot.battery().isFull()) {
                int charge_amount = CHARGE_RATE;
                if (robot.battery().level() + charge_amount > robot.battery().maxLevel()) {
                    charge_amount = robot.battery().maxLevel() - robot.battery().level();
                }
                robot.charge(charge_amount);
                std::cout << "Робот " << robot.id() << " заряжен на " << charge_amount
                          << " (теперь " << robot.battery().level() << "/" << robot.battery().maxLevel() << ")\n";
                if (robot.battery().isFull()) {
                    robot.setCharging(false);
                    std::cout << "Робот " << robot.id() << " полностью заряжен!\n";
                }
            }
        }
    }
    print_map();
}

void setup_manual(WarehouseMap& map, std::vector<Robot>& robots) {
    std::cout << "\n=== Ручная настройка склада ===\n";
    int pickup_x = read_int("Координата X пункта выдачи: ", 0, map.width() - 1);
    int pickup_y = read_int("Координата Y пункта выдачи: ", 0, map.height() - 1);
    map.setPickupPosition({pickup_x, pickup_y});

    int charge_x = read_int("Координата X станции зарядки: ", 0, map.width() - 1);
    int charge_y = read_int("Координата Y станции зарядки: ", 0, map.height() - 1);
    map.setChargingPosition({charge_x, charge_y});

    int shelf_count = read_int("Количество стеллажей (0..20): ", 0, 20);
    for (int i = 0; i < shelf_count; ++i) {
        std::cout << "\n--- Стеллаж #" << (i + 1) << " ---\n";
        int sx = read_int("  Координата X: ", 0, map.width() - 1);
        int sy = read_int("  Координата Y: ", 0, map.height() - 1);
        if (!map.isPassable({sx, sy})) {
            std::cout << "  Клетка непроходима, пропускаем.\n";
            continue;
        }
        int stock = read_int("  Количество товара: ", 1, 999);
        map.setCell({sx, sy}, Cell(CellType::Shelf, stock));
    }

    int wall_count = read_int("Количество стен (0..20): ", 0, 20);
    for (int i = 0; i < wall_count; ++i) {
        std::cout << "\n--- Стена #" << (i + 1) << " ---\n";
        int wx = read_int("  Координата X: ", 0, map.width() - 1);
        int wy = read_int("  Координата Y: ", 0, map.height() - 1);
        map.setCell({wx, wy}, Cell(CellType::Wall));
    }

    int robot_count = read_int("Количество роботов (1..5): ", 1, 5);
    for (int i = 0; i < robot_count; ++i) {
        std::cout << "\n--- Робот #" << (i + 1) << " ---\n";
        int rx = read_int("  Координата X: ", 0, map.width() - 1);
        int ry = read_int("  Координата Y: ", 0, map.height() - 1);
        if (!map.isPassable({rx, ry})) {
            std::cout << "  Клетка непроходима, ставлю на (1,1).\n";
            rx = 1; ry = 1;
        }
        int max_battery = read_int("  Максимальный заряд (10..200): ", 10, 200);
        int capacity = read_int("  Грузоподъёмность (1..50): ", 1, 50);
        robots.emplace_back(i + 1, Coordinate{rx, ry}, max_battery, capacity);
        std::cout << "  Робот создан: (" << rx << "," << ry << "), батарея=" << max_battery
                  << ", вместимость=" << capacity << "\n";
    }
}

void setup_auto(WarehouseMap& map, std::vector<Robot>& robots) {
    std::cout << "\n=== Настройка автоматического режима ===\n";
    int pickup_x = read_int("Координата X пункта выдачи: ", 0, map.width() - 1);
    int pickup_y = read_int("Координата Y пункта выдачи: ", 0, map.height() - 1);
    map.setPickupPosition({pickup_x, pickup_y});

    int charge_x = read_int("Координата X станции зарядки: ", 0, map.width() - 1);
    int charge_y = read_int("Координата Y станции зарядки: ", 0, map.height() - 1);
    map.setChargingPosition({charge_x, charge_y});

    int shelf_count = read_int("Количество стеллажей (1..20): ", 1, 20);
    for (int i = 0; i < shelf_count; ++i) {
        std::cout << "\n--- Стеллаж #" << (i + 1) << " ---\n";
        int sx = read_int("  Координата X: ", 0, map.width() - 1);
        int sy = read_int("  Координата Y: ", 0, map.height() - 1);
        if (!map.isPassable({sx, sy})) {
            std::cout << "  Клетка непроходима, пропускаем.\n";
            continue;
        }
        int stock = read_int("  Количество товара: ", 1, 999);
        map.setCell({sx, sy}, Cell(CellType::Shelf, stock));
    }

    int wall_count = read_int("Количество стен (0..20): ", 0, 20);
    for (int i = 0; i < wall_count; ++i) {
        std::cout << "\n--- Стена #" << (i + 1) << " ---\n";
        int wx = read_int("  Координата X: ", 0, map.width() - 1);
        int wy = read_int("  Координата Y: ", 0, map.height() - 1);
        map.setCell({wx, wy}, Cell(CellType::Wall));
    }

    int robot_count = read_int("Количество роботов (1..5): ", 1, 5);
    for (int i = 0; i < robot_count; ++i) {
        std::cout << "\n--- Робот #" << (i + 1) << " ---\n";
        int rx = read_int("  Координата X: ", 0, map.width() - 1);
        int ry = read_int("  Координата Y: ", 0, map.height() - 1);
        if (!map.isPassable({rx, ry})) {
            std::cout << "  Клетка непроходима, ставлю на (1,1).\n";
            rx = 1; ry = 1;
        }
        int max_battery = read_int("  Максимальный заряд (10..200): ", 10, 200);
        int capacity = read_int("  Грузоподъёмность (1..50): ", 1, 50);
        robots.emplace_back(i + 1, Coordinate{rx, ry}, max_battery, capacity);
        std::cout << "  Робот создан: (" << rx << "," << ry << "), батарея=" << max_battery
                  << ", вместимость=" << capacity << "\n";
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    std::cout << "=== Симулятор склада ===\n\n";
    std::cout << "Выберите режим работы:\n";
    std::cout << "  1 - Ручное управление (сами назначаете заявки и двигаете роботов)\n";
    std::cout << "  2 - Автоматический (диспетчер управляет роботами)\n";
    int mode_choice = read_int("Ваш выбор (1 или 2): ", 1, 2);
    g_mode = (mode_choice == 1) ? Mode::Manual : Mode::Auto;

    int width = read_int("Введите ширину склада (3..30): ", 3, 30);
    int height = read_int("Введите высоту склада (3..30): ", 3, 30);

    WarehouseMap map(width, height);
    g_map = &map;

    std::vector<Robot> robots;

    if (g_mode == Mode::Manual) {
        setup_manual(map, robots);
    } else {
        setup_auto(map, robots);
    }

    Simulator sim(std::move(map), std::move(robots));
    g_sim = &sim;
    g_map = const_cast<WarehouseMap*>(&sim.map());
    g_robots = const_cast<std::vector<Robot>*>(&sim.robots());

    std::cout << "\nСклад инициализирован: " << width << "x" << height << "\n";
    std::cout << "Режим: " << (g_mode == Mode::Manual ? "Ручной" : "Автоматический (диспетчер)") << "\n";
    std::cout << "Введите 'help' для списка команд.\n";
    print_map();

    std::string line;
    while (true) {
        std::cout << "sim> ";
        if (!std::getline(std::cin, line)) break;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd.empty()) continue;
        if (cmd == "quit" || cmd == "exit" || cmd == "выход") break;

        try {
            if (cmd == "help" || cmd == "помощь") {
                std::cout << "Общие команды:\n"
                          << "  tick                        - продвинуть время на 1 такт (авто)\n"
                          << "  charge_tick                 - зарядить роботов на станциях (ручной)\n"
                          << "  map                         - показать карту\n"
                          << "  robots                      - статус роботов\n"
                          << "  orders                      - список заявок\n"
                          << "  create_order <x> <y> <кол-во> - создать заявку (только на стеллаж!)\n"
                          << "  assign <order_id> <robot_id>  - назначить заявку роботу\n"
                          << "  charts                      - открыть графики в окне gnuplot (3 панели)\n"
                          << "  save_charts                 - сохранить данные графиков в CSV\n"
                          << "  journal                     - показать журнал событий\n"
                          << "  save_journal <файл>         - сохранить журнал\n"
                          << "  load_journal <файл>         - загрузить журнал\n";
                if (g_mode == Mode::Manual) {
                    std::cout << "\nКоманды ручного режима:\n"
                              << "  move <id> <dir>             - двигать робота\n"
                              << "  load <id> <кол-во> [dir]    - загрузить со стеллажа\n"
                              << "  unload <id> <кол-во>        - выгрузить в пункте выдачи\n"
                              << "  charge <id>                 - начать зарядку\n"
                              << "  uncharge <id>               - остановить зарядку\n";
                }
            }
            else if (cmd == "tick") {
                if (g_mode == Mode::Auto) {
                    g_sim->tick();
                    std::cout << "Тик " << g_sim->currentTick() << " выполнен\n";
                    print_map();
                } else {
                    std::cout << "В ручном режиме используйте 'charge_tick' для зарядки\n";
                }
            }
            else if (cmd == "charge_tick") {
                charge_tick();
            }
            else if (cmd == "map") {
                print_map();
            }
            else if (cmd == "robots") {
                print_robots();
            }
            else if (cmd == "orders") {
                print_orders();
            }
            else if (cmd == "create_order") {
                int x, y, qty;
                if (iss >> x >> y >> qty) {
                    g_sim->createOrder({x, y}, qty);
                    std::cout << "Заявка создана: стеллаж (" << x << "," << y
                              << "), количество: " << qty << "\n";
                } else {
                    std::cout << "Использование: create_order <x> <y> <кол-во>\n";
                }
            }
            else if (cmd == "assign") {
                int order_id, robot_id;
                if (iss >> order_id >> robot_id) {
                    g_sim->assignOrderToRobot(order_id, robot_id);
                    std::cout << "Заявка " << order_id << " назначена роботу " << robot_id << "\n";
                } else {
                    std::cout << "Использование: assign <order_id> <robot_id>\n";
                }
            }
            else if (cmd == "charts") {
                ChartGenerator::showAllCharts(g_sim->stats());
            }
            else if (cmd == "save_charts") {
                ChartGenerator::saveToCSV(g_sim->stats(), "charts");
                std::cout << "Данные сохранены: charts_queue.csv, charts_completed.csv, charts_charges.csv\n";
            }
            else if (cmd == "journal") {
                const auto& events = g_sim->journal().events();
                if (events.empty()) {
                    std::cout << "Журнал пуст.\n";
                } else {
                    std::cout << "\n--- Журнал событий ---\n";
                    for (const auto& e : events) {
                        std::cout << "[" << e.tick << "] " << e.type << ": " << e.description << "\n";
                    }
                    std::cout << "----------------------\n";
                }
            }
            else if (cmd == "save_journal") {
                std::string filename;
                if (iss >> filename) {
                    g_sim->saveLog(filename);
                    std::cout << "Журнал сохранён в " << filename << "\n";
                } else {
                    std::cout << "Использование: save_journal <файл>\n";
                }
            }
            else if (cmd == "load_journal") {
                std::string filename;
                if (iss >> filename) {
                    g_sim->loadLog(filename);
                    std::cout << "Журнал загружен из " << filename << "\n";
                } else {
                    std::cout << "Использование: load_journal <файл>\n";
                }
            }
            else if (g_mode == Mode::Manual) {
                if (cmd == "move") {
                    int id; std::string dir_str;
                    iss >> id >> dir_str;
                    Robot* r = nullptr;
                    for (auto& rob : *g_robots) if (rob.id() == id) r = &rob;
                    if (!r) throw std::runtime_error("Робот не найден");
                    if (r->isCharging()) throw std::runtime_error("Робот заряжается");
                    Coordinate next = r->position().step(parse_direction(dir_str));
                    if (!g_map->isValid(next)) throw std::runtime_error("Выход за границы карты");
                    if (!g_map->isPassable(next)) throw std::runtime_error("Клетка непроходима");
                    for (const auto& other : *g_robots) {
                        if (other.id() != r->id() && other.position() == next)
                            throw std::runtime_error("Клетка занята другим роботом");
                    }
                    if (!r->able2move()) {
                        throw std::runtime_error("Недостаточно энергии (нужно " +
                                                 std::to_string(r->moveCost()) + ", есть " +
                                                 std::to_string(r->battery().level()) + ")");
                    }
                    r->moveTo(next);
                    std::cout << "Робот " << id << " перемещён на (" << next.x() << "," << next.y() << ")\n";
                    print_map();
                }
                else if (cmd == "load") {
                    int id, amt;
                    iss >> id >> amt;
                    Robot* r = nullptr;
                    for (auto& rob : *g_robots) if (rob.id() == id) r = &rob;
                    if (!r) throw std::runtime_error("Робот не найден");
                    if (r->isCharging()) throw std::runtime_error("Робот заряжается");
                    auto shelves = find_adjacent_shelves(*r);
                    if (shelves.empty()) throw std::runtime_error("Рядом нет стеллажа с товаром");
                    AdjacentShelf* chosen = nullptr;
                    if (shelves.size() == 1) {
                        chosen = &shelves[0];
                    } else {
                        std::string dir_str;
                        if (iss >> dir_str) {
                            Direction dir = parse_direction(dir_str);
                            for (auto& s : shelves) if (s.dir == dir) { chosen = &s; break; }
                            if (!chosen) throw std::runtime_error("Стеллаж в направлении '" + dir_str + "' не найден");
                        } else {
                            std::cout << "Рядом несколько стеллажей. Укажите направление:\n";
                            for (size_t i = 0; i < shelves.size(); ++i) {
                                std::cout << "  " << (i + 1) << ") "
                                          << direction_to_string(shelves[i].dir)
                                          << " — стеллаж на (" << shelves[i].pos.x() << ","
                                          << shelves[i].pos.y() << "), товар: "
                                          << shelves[i].cell->stock() << "\n";
                            }
                            std::cout << "Введите номер или направление: ";
                            std::string choice;
                            std::getline(std::cin, choice);
                            try {
                                int num = std::stoi(choice);
                                if (num >= 1 && num <= (int)shelves.size()) chosen = &shelves[num - 1];
                                else throw std::runtime_error("Неверный номер");
                            } catch (...) {
                                Direction dir = parse_direction(choice);
                                for (auto& s : shelves) if (s.dir == dir) { chosen = &s; break; }
                                if (!chosen) throw std::runtime_error("Неверное направление");
                            }
                        }
                    }
                    if (chosen->cell->stock() < amt) throw std::runtime_error("Недостаточно товара (есть " + std::to_string(chosen->cell->stock()) + ")");
                    if (!r->Carry(amt)) throw std::runtime_error("Робот не может столько нести (свободно " + std::to_string(r->freeCapacity()) + ")");
                    if (!r->battery().hasEnough(amt)) throw std::runtime_error("Недостаточно энергии для погрузки");
                    chosen->cell->removeStock(amt);
                    r->load(amt);
                    r->spendEnergy(amt);
                    std::cout << "Загружено " << amt << " ед. со стеллажа (" << chosen->pos.x() << "," << chosen->pos.y() << ")\n";
                    print_map();
                }
                else if (cmd == "unload") {
                    int id, amt;
                    iss >> id >> amt;
                    Robot* r = nullptr;
                    for (auto& rob : *g_robots) if (rob.id() == id) r = &rob;
                    if (!r) throw std::runtime_error("Робот не найден");
                    Cell& cell = g_map->cellAt(r->position());
                    if (!cell.isPickup()) throw std::runtime_error("Робот должен быть в пункте выдачи (P)");
                    if (r->deliver() < amt) throw std::runtime_error("У робота недостаточно груза");
                    if (!r->battery().hasEnough(amt)) throw std::runtime_error("Недостаточно энергии");
                    r->unload(amt);
                    r->spendEnergy(amt);
                    std::cout << "Выгружено " << amt << " ед. товара в пункт выдачи\n";
                    print_map();
                }
                else if (cmd == "charge") {
                    int id;
                    iss >> id;
                    Robot* r = nullptr;
                    for (auto& rob : *g_robots) if (rob.id() == id) r = &rob;
                    if (!r) throw std::runtime_error("Робот не найден");
                    Cell& cell = g_map->cellAt(r->position());
                    if (!cell.isChargingStation()) throw std::runtime_error("Робот должен быть на зарядке (C)");
                    if (r->battery().isFull()) {
                        std::cout << "Робот " << id << " уже полностью заряжен\n";
                    } else {
                        r->setCharging(true);
                        std::cout << "Робот " << id << " начал зарядку. Используйте 'charge_tick' для продвижения зарядки\n";
                    }
                }
                else if (cmd == "uncharge") {
                    int id;
                    iss >> id;
                    Robot* r = nullptr;
                    for (auto& rob : *g_robots) if (rob.id() == id) r = &rob;
                    if (!r) throw std::runtime_error("Робот не найден");
                    if (!r->isCharging()) throw std::runtime_error("Робот не заряжается");
                    r->setCharging(false);
                    std::cout << "Робот " << id << " отключён от зарядки (батарея: "
                              << r->battery().level() << "/" << r->battery().maxLevel() << ")\n";
                }
                else {
                    std::cout << "Неизвестная команда. Введите 'help'.\n";
                }
            }
            else {
                std::cout << "Неизвестная команда. Введите 'help'.\n";
            }
        }
        catch (const DomainError& e) {
            std::cerr << "[Ошибка домена]: " << e.what() << "\n";
        }
        catch (const std::exception& e) {
            std::cerr << "[Ошибка]: " << e.what() << "\n";
        }
    }

    return 0;
}