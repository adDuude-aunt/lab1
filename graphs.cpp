#include "graphs.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <cstdlib>

namespace warehouse {

static bool isGnuplotAvailable() {
    int ret = system("gnuplot --version >nul 2>&1");
    return ret == 0;
}

std::string ChartGenerator::renderBarChart(const std::string& title,
                                            const std::vector<int>& values,
                                            int width) {
    if (values.empty()) return title + "\n(нет данных)\n";
    std::ostringstream out;
    out << title << "\n" << std::string(title.size(), '=') << "\n";
    int maxVal = *std::max_element(values.begin(), values.end());
    if (maxVal == 0) maxVal = 1;
    for (size_t i = 0; i < values.size(); ++i) {
        int barLen = static_cast<int>(values[i] * width / maxVal);
        out << std::setw(4) << i << " |";
        for (int j = 0; j < barLen; ++j) out << '#';
        out << " " << values[i] << "\n";
    }
    return out.str();
}

std::string ChartGenerator::renderMultiLineChart(const std::string& title,
                                                  const std::vector<std::vector<int>>& series,
                                                  const std::vector<std::string>& labels) {
    if (series.empty()) return title + "\n(нет данных)\n";
    std::ostringstream out;
    out << title << "\n" << std::string(title.size(), '=') << "\n";
    int maxTicks = 0;
    for (const auto& s : series) maxTicks = std::max(maxTicks, (int)s.size());
    out << std::setw(6) << "Tick";
    for (const auto& label : labels) out << std::setw(10) << label;
    out << "\n" << std::string(6 + 10 * labels.size(), '-') << "\n";
    for (int t = 0; t < maxTicks; ++t) {
        out << std::setw(6) << t;
        for (const auto& s : series) {
            if (t < (int)s.size()) out << std::setw(10) << s[t];
            else out << std::setw(10) << "-";
        }
        out << "\n";
    }
    return out.str();
}

void ChartGenerator::saveCharts(const std::vector<TickStats>& stats,
                                 const std::string& prefix) {
    if (stats.empty()) return;
    std::vector<int> pending, completed;
    for (const auto& s : stats) {
        pending.push_back(s.pendingOrders);
        completed.push_back(s.completedOrders);
    }
    std::ofstream f1(prefix + "_queue.txt");
    if (f1) f1 << renderBarChart("Длина очереди заявок", pending);
    std::ofstream f2(prefix + "_completed.txt");
    if (f2) f2 << renderBarChart("Выполненные заявки", completed);
    int robotCount = stats[0].robotCharges.size();
    std::vector<std::vector<int>> series(robotCount);
    std::vector<std::string> labels(robotCount);
    for (int r = 0; r < robotCount; ++r) {
        labels[r] = "Robot" + std::to_string(r + 1);
        for (const auto& s : stats)
            if (r < (int)s.robotCharges.size())
                series[r].push_back(s.robotCharges[r]);
    }
    std::ofstream f3(prefix + "_charges.txt");
    if (f3) f3 << renderMultiLineChart("Заряд роботов по тактам", series, labels);
}

void ChartGenerator::saveToCSV(const std::vector<TickStats>& stats,
                                const std::string& prefix) {
    if (stats.empty()) return;

    {
        std::ofstream f(prefix + "_queue.csv");
        f << "Tick,PendingOrders\n";
        for (const auto& s : stats)
            f << s.tick << "," << s.pendingOrders << "\n";
    }
    {
        std::ofstream f(prefix + "_completed.csv");
        f << "Tick,CompletedOrders\n";
        for (const auto& s : stats)
            f << s.tick << "," << s.completedOrders << "\n";
    }
    {
        std::ofstream f(prefix + "_charges.csv");
        f << "Tick";
        int robotCount = stats[0].robotCharges.size();
        for (int r = 0; r < robotCount; ++r)
            f << ",Robot" << (r + 1);
        f << "\n";
        for (const auto& s : stats) {
            f << s.tick;
            for (int r = 0; r < robotCount; ++r)
                f << "," << (r < (int)s.robotCharges.size()
                             ? std::to_string(s.robotCharges[r]) : "");
            f << "\n";
        }
    }
}

void ChartGenerator::showAllCharts(const std::vector<TickStats>& stats) {
    if (stats.empty()) {
        std::cout << "Нет данных для графиков. Выполните хотя бы один тик.\n";
        return;
    }


    int robotCount = stats[0].robotCharges.size();
    std::string dataFile = "tmp_chart.dat";

    {
        std::ofstream f(dataFile);
        for (const auto& s : stats) {
            f << s.tick << " " << s.pendingOrders << " " << s.completedOrders;
            for (int r = 0; r < robotCount; ++r)
                f << " " << (r < (int)s.robotCharges.size() ? s.robotCharges[r] : 0);
            f << "\n";
        }
    }


    std::ostringstream plotRobots;
    for (int r = 0; r < robotCount; ++r) {
        if (r > 0) plotRobots << ", ";
        plotRobots << "'" << dataFile << "' using 1:" << (r + 4)
                   << " with lines lc rgb 'black' title 'Robot " << (r + 1) << "'";
    }

    std::string scriptFile = "tmp_chart.gp";
    {
        std::ofstream f(scriptFile);
        
        f << "set term wxt 0 enhanced size 800,400\n";
        f << "set title 'Pending orders (queue length)'\n";
        f << "set xlabel 'Tick'\n";
        f << "set ylabel 'Pending'\n";
        f << "set grid\n";
        f << "plot '" << dataFile << "' using 1:2 with lines lc rgb 'black' title 'Pending'\n";

        f << "set term wxt 1 enhanced size 800,400\n";
        f << "set title 'Completed orders'\n";
        f << "set xlabel 'Tick'\n";
        f << "set ylabel 'Completed'\n";
        f << "set grid\n";
        f << "plot '" << dataFile << "' using 1:3 with lines lc rgb 'black' title 'Completed'\n";


        f << "set term wxt 2 enhanced size 800,400\n";
        f << "set title 'Robot battery charge per tick'\n";
        f << "set xlabel 'Tick'\n";
        f << "set ylabel 'Charge'\n";
        f << "set grid\n";
        f << "plot " << plotRobots.str() << "\n";
    }

#ifdef _WIN32
    std::string cmd = "start \"\" gnuplot -persist " + scriptFile;
#else
    std::string cmd = "gnuplot -persist " + scriptFile + " &";
#endif
    int ret = system(cmd.c_str());
    if (ret == 0) {
        std::cout << "Графики открыты в 3-х окнах.\n";
    } else {
        std::cout << "Не удалось запустить gnuplot.\n";
    }
}

}