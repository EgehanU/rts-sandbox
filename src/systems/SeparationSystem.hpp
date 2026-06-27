#pragma once
#include "raylib.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include "world/Grid.hpp"
#include <cmath>    
#include <vector>

namespace rts{

// pushes units apart so they donr stack
inline void update_separation(Registry& reg, const Grid& grid, float dt){
    const float MIN_DIST = 18.0f;     
    const float PUSH = 80.0f;       
    // grab the position pool once, need to compare every pair packed arrays 
    auto& positions = reg.pool<Position>();
    auto& data = positions.data();
    int n =(int)data.size();

    // build the pushes first, apply after
    std::vector<Vector2> push(n,{0, 0});

    for (int i = 0; i < n; i++){
        for (int j = i + 1; j < n; j++){ // j starts at i+1 so each pair once
            float dx = data[i].x- data[j].x;
            float dy = data[i].y- data[j].y;
            float dist = sqrtf(dx*dx + dy*dy);

            if (dist < MIN_DIST && dist > 0.001f){
                float nx = dx / dist;
                float ny = dy / dist;
                // closer = stronger shove, (MIN_DIST-dist) grows as they overlap
                float strength = (MIN_DIST-dist);
                push[i].x += nx * strength;
                push[i].y += ny * strength;
                push[j].x -= nx * strength;   
                push[j].y -= ny * strength;
            }
        }
    }

    //apply all the accumulated pushes, but capped so a unit cant get launched across the screen in one frme
    const float MAX_PUSH = 40.0f;  
    for (int i = 0; i < n; i++){
        float px = push[i].x * PUSH * dt;
        float py = push[i].y * PUSH * dt;

        // clamp the push length so big overlaps dont produce huge shoves
        float len = sqrtf(px*px + py*py);
        float maxStep = MAX_PUSH * dt;
        if (len > maxStep && len > 0.001f){
            px = (px / len) * maxStep;
            py = (py / len) * maxStep;
        }

        //where this push would land the unit
        float newX = data[i].x + px;
        float newY = data[i].y + py;

        if (!grid.is_blocked(grid.world_to_cell_x(newX), grid.world_to_cell_y(data[i].y)))
            data[i].x = newX;
        if (!grid.is_blocked(grid.world_to_cell_x(data[i].x), grid.world_to_cell_y(newY)))
            data[i].y = newY;
    }
}

}