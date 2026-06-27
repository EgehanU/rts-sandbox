#pragma once
#include "raylib.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include "world/Grid.hpp"
#include <vector>
#include <cstdint>

namespace rts {

enum FogState : std::uint8_t{ // classic, first seen this at empire earth 2 or age of empires, if not played please do it:)
    FOG_HIDDEN = 0, // never seen, black
    FOG_EXPLORED = 1, // seen before, dim
    FOG_VISIBLE = 2 // a unit sees it right now, lit
};

// holds one fog state per grid cell. main owns one of these, same size as the grid, and gives it to the system each frame
struct FogMap{
    int width = 0,height = 0;
    std::vector<std::uint8_t> cells;

    FogMap() = default;
    FogMap(int w, int h) : width(w), height(h) {
        cells.resize(w *h, FOG_HIDDEN); // everthing hidden
    }

    std::uint8_t& at(int cx, int cy){
        return cells[cy * width +cx]; 
    }
    std::uint8_t  at(int cx, int cy) const{
        return cells[cy * width +cx]; 
    }
};

// Anything currently visible = explored, light up cells around every unit that has vision, which requires position and vision
inline void update_fog(Registry& reg, const Grid& grid, FogMap& fog){
    for (auto& c : fog.cells){
        if (c == FOG_VISIBLE) 
            c = FOG_EXPLORED;
    }

    reg.view<Position, Vision>([&](Entity,Position& p, Vision& v){
        // how many cells the vision radius covers
        int cellRadius = (int)(v.radius/ grid.cellSize);
        int ucx = grid.world_to_cell_x(p.x);
        int ucy = grid.world_to_cell_y(p.y);

        // scan the square around the unit, but only light cells actually
        for (int oy = -cellRadius; oy <= cellRadius; oy++){
            for (int ox = -cellRadius; ox<= cellRadius; ox++){
                int cx = ucx + ox;
                int cy = ucy + oy;
                if (!grid.in_bounds(cx, cy)) continue;

                // circle check: if ox^2 + oy^2 <= r^2
                if (ox*ox + oy*oy <= cellRadius*cellRadius){
                    fog.at(cx, cy) = FOG_VISIBLE;
                }
            }
        }
    });
}

// draw the fog overlay, call insde the camera sandwich, after the units so it coverers them. hidden = solid black, explored = translucent dark, visible = nothing
inline void render_fog(const Grid& grid, const FogMap& fog){
    for (int cy = 0; cy < grid.height; cy++){
        for (int cx = 0; cx < grid.width; cx++){
            std::uint8_t state = fog.at(cx, cy);
            if (state == FOG_VISIBLE) continue;   // lit, draw no overlay

            Color shade = (state == FOG_EXPLORED)
                ? Color{0, 0, 0, 120} // semitransparent
                : Color{0, 0, 0, 245}; // almost opaque

            DrawRectangle((int)(cx * grid.cellSize),
                          (int)(cy * grid.cellSize),
                          (int)grid.cellSize,
                          (int)grid.cellSize,
                          shade);
        }
    }
}

}