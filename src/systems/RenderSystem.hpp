#pragma once
#include "raylib.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include "world/Grid.hpp"
namespace rts{

// draws every unit as a circle, selected ones in a different color.
// assumes BeginMode2D was already called
inline void render_units(Registry& reg){
    reg.view<Position, Selectable>([](Entity, Position& p, Selectable& s){
        Color c = s.selected ? ORANGE : DARKBLUE;
        DrawCircle((int)p.x, (int)p.y, 8, c);
        if (s.selected) DrawCircleLines((int)p.x, (int)p.y, 11, ORANGE);
    });
}

// draw every blocked cell as a gray square
inline void render_grid(const Grid& grid){
    for (int cy = 0;cy < grid.height; cy++){
        for (int cx = 0; cx < grid.width; cx++){
            if (grid.is_blocked(cx, cy)){
                DrawRectangle((int)(cx * grid.cellSize),
                              (int)(cy * grid.cellSize),
                              (int)grid.cellSize,
                              (int)grid.cellSize,
                              GRAY);
            }
        }
    }
}

}