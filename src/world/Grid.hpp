#pragma once
#include <vector>
#include <cstdint>

namespace rts{

// the world described into square cells. each cell is either open or blocked
// pathfinding and fog of war will be read from this grid
struct Grid{
    int width = 0; // cells across
    int height = 0; // cells down
    float cellSize = 32.0f; //how many world units one cell spans

    // one byte per cell, 1 = blocked, 0 = open. flat array, not 2d
    std::vector<std::uint8_t> blocked;

    Grid() = default;

    Grid(int w, int h, float cs) : width(w), height(h), cellSize(cs){
        blocked.resize(w * h, 0);   // everything open to start
    }

    // turn a 2d cell coord into the flat array index
    // row y times width + column x. 2d->1d trick
    int index(int cx, int cy) const{
        return cy * width + cx;
    }

    bool in_bounds(int cx, int cy) const{
        return cx >= 0 && cx < width && cy >= 0 && cy < height;
    }

    bool is_blocked(int cx, int cy) const{
        if (!in_bounds(cx,cy)) 
            return true;
        return blocked[index(cx, cy)] != 0;
    }

    void set_blocked(int cx, int cy, bool b){
        if (in_bounds(cx, cy)) blocked[index(cx, cy)] = b ? 1 : 0;
    }

    // world position, which cell it falls in
    int world_to_cell_x(float wx) const{ 
        return (int)(wx / cellSize); 
    }
    int world_to_cell_y(float wy) const{ 
        return (int)(wy / cellSize); 
    }

    // not corners, so they walk down the middle of open lanes
    float cell_to_world_x(int cx) const { return (cx + 0.5f) * cellSize; }
    float cell_to_world_y(int cy) const { return (cy + 0.5f) * cellSize; }
};
// makes a copy of the grid with every wall fattened by margin cells.
// pathfinding runs on this fattened version
inline Grid inflate_obstacles(const Grid& src, int margin) {
    Grid out = src;  

    for (int cy = 0; cy < src.height; cy++) {
        for (int cx = 0; cx < src.width; cx++) {
            if (!src.is_blocked(cx, cy)) continue;   // only grow around real walls

            // block every cell within margin of this wall cell
            for (int oy= -margin; oy <= margin; oy++) {
                for (int ox = -margin; ox <= margin; ox++) {
                    out.set_blocked(cx+ ox, cy + oy, true);
                }
            }
        }
    }
    return out;
}

}