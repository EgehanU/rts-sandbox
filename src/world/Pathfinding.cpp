#include "world/Pathfinding.hpp"
#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>

namespace rts{

// A* works on grid cells
// g = real cost to get here from the start
// f = g + h, where h is the guess of remaining cost to the goal.

// one entry in the priority queue. we order by f (smaller f = higher priority)
struct Node {
    int cell;     
    float f;
    // priority_queue is a max-heap by default, so we flip the comparison
    // to make it pop the smallest f first
    bool operator>(const Node& other) const{
        return f > other.f; 
    }
};

static float heuristic(int x1, int y1, int x2, int y2){
    int dx = std::abs(x1 - x2);
    int dy = std::abs(y1 - y2);
    // diagonal steps cost ~1.414, straight steps cost 1. this formula is the
    // exact cost assuming no walls, which makes A* both fast and correct
    return (dx + dy) + (1.414f- 2.0f)* std::min(dx, dy);
}

// if a cell is blocked, go outward to find the closest open one.
static int nearest_open_cell(const Grid& grid, int cx, int cy){
    if (!grid.is_blocked(cx, cy)) return grid.index(cx, cy);

    // search rings of increasing radius, untiul reaching an open cell
    for (int r = 1; r< 20; r++){
        for (int oy = -r; oy <= r; oy++){
            for (int ox = -r; ox <= r; ox++){
                if (std::abs(ox) != r && std::abs(oy) != r) continue;
                int nx= cx + ox;
                int ny = cy + oy;
                if (!grid.is_blocked(nx, ny))
                    return grid.index(nx, ny);
            }
        }
    }
    return -1;   //nothing open nearby, give up
}

std::vector<PathPoint> find_path(const Grid& grid, float startX, float startY,float goalX, float goalY) {
    int startCx =grid.world_to_cell_x(startX);
    int startCy =grid.world_to_cell_y(startY);
    int goalCx  =grid.world_to_cell_x(goalX);
    int goalCy  =grid.world_to_cell_y(goalY);

    // clamp the start the same way as the goal. is_blocked is true for
    // off-grid cells too, so this also catches a start outside the map
    if (grid.is_blocked(startCx, startCy)){
        int open = nearest_open_cell(grid, startCx, startCy);
        if (open==-1) return {};
        startCx =open % grid.width;
        startCy =open / grid.width;
    }

        if (grid.is_blocked(goalCx, goalCy)){
            int open = nearest_open_cell(grid, goalCx, goalCy);
            if (open==-1) return{};
            goalCx = open % grid.width;
            goalCy = open / grid.width;
    }

    int total = grid.width * grid.height;
    int startCell = grid.index(startCx, startCy);
    int goalCell= grid.index(goalCx, goalCy);

    // g score for every cell
    std::vector<float> gScore(total, 1e9f);
    std::vector<int> cameFrom(total, -1);

    gScore[startCell] = 0.0f;

    // min-heap so we always pull the most promising lowest f cell
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    open.push({startCell, heuristic(startCx, startCy, goalCx, goalCy)});

    // the 8 directions a unit can step: 4 straight + 4 diagonal
    int dirX[8] = { 1, -1,  0,  0,  1,  1, -1, -1 };
    int dirY[8] = { 0,  0,  1, -1,  1, -1,  1, -1 };

    while (!open.empty()) {
        int current = open.top().cell;
        open.pop();

        if (current == goalCell) break;

        int cx = current % grid.width;   // flat index back into x,y
        int cy = current /grid.width;

        // look at all 8 neighbors
        for (int d = 0; d < 8; d++){
            int nx = cx + dirX[d];
            int ny = cy + dirY[d];

            if (grid.is_blocked(nx, ny)) continue;   // wall or off-grid, skip

            bool diagonal = (dirX[d] != 0 && dirY[d] != 0);

            // a diagonal move passes through the corner where two cells meet
            // if both of those cells are walls, the unit would clip the corner,
            // so dont allow that diagonal
            if (diagonal &&(grid.is_blocked(cx + dirX[d], cy) || grid.is_blocked(cx, cy + dirY[d])))
                continue;

            //diagonal moves cost more than straight ones
            float stepCost = diagonal ? 1.414f : 1.0f;

            int neighbor = grid.index(nx, ny);
            float tentative = gScore[current] + stepCost;

            // if a cheaper way found
            if (tentative < gScore[neighbor]){
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative;
                float f = tentative + heuristic(nx, ny, goalCx, goalCy);
                open.push({neighbor, f});
            }
        }
    }

    if (cameFrom[goalCell] == -1 && startCell != goalCell) return {};

    std::vector<PathPoint> path;
    int cell = goalCell;
    while (cell != -1) {
        int cx = cell % grid.width;
        int cy = cell / grid.width;
        path.push_back({ grid.cell_to_world_x(cx), grid.cell_to_world_y(cy) });
        if (cell == startCell) break;
        cell = cameFrom[cell];
    }
    std::reverse(path.begin(), path.end());   // goal to start became start to goal

    return path;
}

}