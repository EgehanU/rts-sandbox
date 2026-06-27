#pragma once
#include "raylib.h"

namespace rts {

// dead simple profiler: named time slots which filled in each frame
struct Profiler {
    double pathfindingMs = 0;
    double movementMs = 0;
    double separationMs = 0;
    double fogMs = 0;
    double now_ms() const { return GetTime() * 1000.0; }
};

}