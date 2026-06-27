#pragma once
#include "raylib.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include "world/Grid.hpp"
#include "world/Pathfinding.hpp"

namespace rts {

// right click: ask A* for a route for each selected unit, around the walls
// needs the grid now so pathfinding knows where the obstacles are
inline void update_orders(Registry& reg,const Camera2D& cam, const Grid& grid){
    if (!IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) return;

    Vector2 target = GetScreenToWorld2D(GetMousePosition(), cam);

    //each selected unit gets its own path from where it is to the target
    reg.view<Position, Selectable>([&](Entity e, Position& p, Selectable& s){
        if (!s.selected) return;
        if (!reg.has<Path>(e)) return;

        // run A* from this units spot to the clicked spot
        auto route = find_path(grid, p.x, p.y, target.x, target.y);

        Path& path = reg.get<Path>(e);
        if(!route.empty()){
            path.points = route;
            path.current = 0;
            path.active = true;
        } 
        else{
            path.active = false;   // no route found
        }
    });
}

}