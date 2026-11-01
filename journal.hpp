#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace warehouse {

    struct Event {
        int tick;
        std::string type;
        std::string description;
    };

    class EventJournal {
        private:
            std::vector<Event> events_;
        public:
            void log(int tick, const std::string& type, const std::string& desc);
            const std::vector<Event>& events() const;
            void save(const std::string& filename) const;
            void load(const std::string& filename);
            void clear();
    };

}