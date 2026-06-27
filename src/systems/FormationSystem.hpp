#pragma once
#include "raylib.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include "world/Grid.hpp"
#include "world/Pathfinding.hpp"
#include <vector>
#include <cmath>

namespace rts {

// called on right-click. spreads the selected units into a grid shape around
// they will shape like a square, therfore they will not come on top of esch other
inline void update_formation_orders(Registry& reg, const Camera2D& cam, const Grid& navGrid) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) return;

    Vector2 target =GetScreenToWorld2D(GetMousePosition(),cam);

    // collect the selected units first. need the count to build the shape
    std::vector<Entity> chosen;
    reg.view<Position, Selectable>([&](Entity e, Position&, Selectable& s){
        if (s.selected) chosen.push_back(e);
    });

    int n = (int)chosen.size();
    if (n == 0) 
        return;

    const float SPACING =24.0f;
    int cols = (int)ceilf(sqrtf((float)n));

    // build the list of slot positions, centered on the click
    std::vector<Vector2> slots;
    for (int i = 0; i < n; i++){
        int row = i / cols;
        int col = i % cols;
        float offsetX = (col -(cols - 1) * 0.5f) * SPACING;
        float offsetY = (row - ((n-1) / cols) * 0.5f) * SPACING;
        slots.push_back({ target.x + offsetX, target.y + offsetY });
    }

    // greedy nearest slot matching: for each slot
    std::vector<bool> unitTaken(n, false);

    for (int s = 0; s < n; s++){
        int bestUnit = -1;
        float bestDist = 1e18f;

        // find the closest free unit to this slot
        for (int u = 0; u < n; u++){
            if (unitTaken[u]) continue;
            Position& p = reg.get<Position>(chosen[u]);
            float dx = p.x -slots[s].x;
            float dy = p.y -slots[s].y;
            float dist = dx*dx + dy*dy;// squared, no sqrt needed for compare
            if (dist<bestDist) {
                bestDist = dist;
                bestUnit = u;
            }
        }

        if (bestUnit == -1) continue;
        unitTaken[bestUnit] = true;

        // path that unit to this slot
        Entity e = chosen[bestUnit];
        if (!reg.has<Path>(e)) continue;

        Position& p = reg.get<Position>(e);
        auto route = find_path(navGrid, p.x, p.y, slots[s].x, slots[s].y);

        Path& path = reg.get<Path>(e);
        if (!route.empty()) {
            path.points = route;
            path.current = 0;
            path.active = true;
        } else {
            path.active = false;
        }
    }
}

}