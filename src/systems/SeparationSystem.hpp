#pragma once
#include "raylib.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include "world/Grid.hpp"
#include <cmath>
#include <vector>
#include <unordered_map>

namespace rts{

// pushes units apart so they dont stack, upgraded to using a spatial hash
// each unit checks units in its on and 8 neighbours 
// O(n^2) to O(n) with the changes
inline void update_separation(Registry& reg, const Grid& grid, float dt){
    const float MIN_DIST = 18.0f;
    const float PUSH = 80.0f;

    auto& positions = reg.pool<Position>();
    auto& data = positions.data();
    int n = (int)data.size();
    if (n == 0) return;

    // cell size = the interaction range
    const float CELL = MIN_DIST;

    // hash a 2d cell coord into one 64-bit key so it fits in a map
    // pack x in the high bits, y in the low bits
    auto cell_key = [CELL](float x, float y) -> long long{
        long long cx = (long long)floorf(x / CELL);
        long long cy = (long long)floorf(y / CELL);
        return (cx << 32) ^ (cy & 0xffffffff);
    };

    // drop every units index into the bucket for its cell
    std::unordered_map<long long, std::vector<int>> buckets;
    buckets.reserve(n);
    for (int i = 0; i < n; i++) {
        buckets[cell_key(data[i].x, data[i].y)].push_back(i);
    }

    std::vector<Vector2> push(n, {0, 0});

    //for each unit, only test units in the 9 surrounding buckets
    for (int i = 0; i < n; i++){
        long long baseCx = (long long)floorf(data[i].x/CELL);
        long long baseCy = (long long)floorf(data[i].y/CELL);

        for (int oy = -1; oy <= 1; oy++) {
            for (int ox = -1; ox <= 1; ox++) {
                long long key = ((baseCx + ox) << 32)^ ((baseCy + oy) & 0xffffffff);
                auto it = buckets.find(key);
                if (it == buckets.end()) continue; // empty cell, skip

                for (int j : it->second) {
                    if (j <= i) 
                        continue; // each pair once, and skip self
                    float dx = data[i].x - data[j].x;
                    float dy = data[i].y - data[j].y;
                    float dist = sqrtf(dx*dx + dy*dy);

                    if (dist < MIN_DIST && dist> 0.001f){
                        float nx = dx / dist;
                        float ny = dy / dist;
                        float strength = (MIN_DIST-dist);
                        push[i].x += nx * strength;
                        push[i].y += ny * strength;
                        push[j].x -= nx * strength;
                        push[j].y -= ny * strength;
                    }
                }
            }
        }
    }

    //dont shove units into walls
    const float MAX_PUSH = 40.0f;
    for (int i = 0; i < n; i++) {
        float px = push[i].x * PUSH * dt;
        float py = push[i].y * PUSH * dt;

        float len = sqrtf(px*px + py*py);
        float maxStep = MAX_PUSH * dt;
        if (len > maxStep && len > 0.001f){
            px = (px /len) * maxStep;
            py = (py /len) * maxStep;
        }

        float newX = data[i].x + px;
        float newY = data[i].y + py;
        if (!grid.is_blocked(grid.world_to_cell_x(newX), grid.world_to_cell_y(data[i].y)))
            data[i].x = newX;
        if (!grid.is_blocked(grid.world_to_cell_x(data[i].x), grid.world_to_cell_y(newY)))
            data[i].y = newY;
    }
}

}