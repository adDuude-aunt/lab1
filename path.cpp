#include "path.hpp"
#include <queue>
#include <algorithm>

namespace warehouse {

    std::vector<Coordinate> Pathfinding::findPath(const Coordinate& start, const Coordinate& end, 
                                                const WarehouseMap& map, 
                                                const std::vector<Coordinate>& obstacles) {
        if (start == end) return {start};
        if (!map.isValid(start) || !map.isValid(end)) return {};

        int w = map.width();
        int h = map.height();
        std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
        std::vector<std::vector<Coordinate>> parent(h, std::vector<Coordinate>(w, Coordinate(-1, -1)));
        
        std::queue<Coordinate> q;
        q.push(start);
        visited[start.y()][start.x()] = true;

        Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};

        while (!q.empty()) {
            Coordinate curr = q.front(); q.pop();
            if (curr == end) break;

            for (auto d : dirs) {
                Coordinate next = curr.step(d);
                if (map.isValid(next) && map.isPassable(next) && !visited[next.y()][next.x()]) {
                    bool isObstacle = false;
                    for (const auto& obs : obstacles) {
                        if (obs == next) { isObstacle = true; break; }
                    }
                    if (!isObstacle) {
                        visited[next.y()][next.x()] = true;
                        parent[next.y()][next.x()] = curr;
                        q.push(next);
                    }
                }
            }
        }

        if (!visited[end.y()][end.x()]) return {};

        std::vector<Coordinate> path;
        Coordinate curr = end;
        while (!(curr == start)) {
            path.push_back(curr);
            curr = parent[curr.y()][curr.x()];
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        return path;
    }

}