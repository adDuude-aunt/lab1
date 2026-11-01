#include "journal.hpp"

namespace warehouse {

    void EventJournal::log(int tick, const std::string& type, const std::string& desc) {
        events_.push_back({tick, type, desc});
    }

    const std::vector<Event>& EventJournal::events() const { return events_; }

    void EventJournal::save(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file) throw std::runtime_error("Cannot open file for writing: " + filename);
        for (const auto& e : events_) {
            file << e.tick << "|" << e.type << "|" << e.description << "\n";
        }
    }

    void EventJournal::load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) throw std::runtime_error("Cannot open file for reading: " + filename);
        events_.clear();
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            Event e;
            std::string tickStr;
            if (std::getline(iss, tickStr, '|') && std::getline(iss, e.type, '|') && std::getline(iss, e.description)) {
                e.tick = std::stoi(tickStr);
                events_.push_back(e);
            }
        }
    }

    void EventJournal::clear() { events_.clear(); }

}