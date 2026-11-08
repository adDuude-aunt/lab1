#pragma once
#include <vector>
#include <string>
#include "simulator.hpp"

namespace warehouse {

class ChartGenerator {
public:
    // ASCII-графики (оставлены для утилитарных задач, но в интерфейсе не используются)
    static std::string renderBarChart(const std::string& title,
                                       const std::vector<int>& values,
                                       int width = 40);

    static std::string renderMultiLineChart(const std::string& title,
                                             const std::vector<std::vector<int>>& series,
                                             const std::vector<std::string>& labels);

    // Сохранение ASCII-графиков в файлы
    static void saveCharts(const std::vector<TickStats>& stats,
                           const std::string& prefix);

    // Сохранить данные в CSV
    static void saveToCSV(const std::vector<TickStats>& stats,
                          const std::string& prefix);

    // Показать все 3 графика в одном окне gnuplot (multiplot 3,1)
    static void showAllCharts(const std::vector<TickStats>& stats);
};

}