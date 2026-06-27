#pragma once
#include "world/Pathfinding.hpp"
#include <vector>
namespace rts{

struct Path{
    std::vector<PathPoint> points;
    int current = 0;       // index of the waypoint that is targeted to
    bool active = false;   // false = idle, no route
};

// where the unit is in the world, in world units(not pixels)
struct Position { float x, y; };

// how fast it is moving, world units per second, sign picks direction
struct Velocity { float x, y; };

// dummy hp for now, nothing actually uses it yet, just here to show
struct Health { float current, max; };

// is this unit currently selected by the mouse
struct Selectable { bool selected = false; };

// where this unit is trying to walk to. active=false means it's idle
struct MoveTarget { float x, y; bool active = false; };

// how far this unit can see, important for fog of war
struct Vision { float radius = 200.0f; };

// what kind of unit, not used at the moment:)
struct UnitType {
    enum Kind { Infantry, Vehicle } kind = Infantry;
};

}