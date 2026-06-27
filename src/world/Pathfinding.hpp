#pragma once
#include <vector>
#include "world/Grid.hpp"

namespace rts {

// a single point on a path, in world coords. units walk from one to the next
struct PathPoint { float x, y; };

// find a route from one world position to another, going around blocked cells.
std::vector<PathPoint> find_path(const Grid& grid,float startX, float startY,float goalX, float goalY);

}