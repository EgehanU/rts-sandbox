#pragma once
#include "raylib.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include <cmath> // sqrtf

namespace rts{

inline void update_movement(Registry& reg, float dt) {
    const float SPEED = 200.0f; // world units per second
    const float ARRIVE_DIST = 4.0f; // close enough to a waypoint

    // walk each unit along its path, need Position and path
    reg.view<Position,Path>([&](Entity, Position& p, Path& path){
        if (!path.active) return;
        if (path.current >= (int)path.points.size()){
            path.active = false;//ran out of waypoints
            return;
        }

        // aim at the current waypoint
        PathPoint wp =path.points[path.current];
        float dx = wp.x -p.x;
        float dy = wp.y -p.y;
        float dist = sqrtf(dx*dx + dy*dy);

        if (dist < ARRIVE_DIST){
            path.current++;
            return;
        }

        // normalized direction * speed * dt, same as before
        float stepX= (dx / dist) * SPEED * dt;
        float stepY= (dy / dist) * SPEED * dt;
        p.x += stepX;
        p.y += stepY;
    });
}

}