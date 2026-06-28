# RTS Sandbox

A small real time strategy sandbox written from scratch in C++20, using [raylib](https://www.raylib.com/) for rendering and [Dear ImGui](https://github.com/ocornut/imgui) for the debug tools.

I worked with Raylib before however, I have not worked on an RTS before, so I built one to learn how the core systems actually work, which consists of selecting units, moving them around obstacles, keeping them in formation, and making the whole thing hold up when there are a few thousand units on the map. It is a tech demo, not a game. There is no win condition or enemy AI. The point is the systems underneath.

Everything here is hand written. The entity system, the pathfinding, the formations, the fog of war, none of it comes from an engine or a library. That was the whole idea, to understand the the logic by building it.

![RTS Sandbox](docs/img_game.png)

## How it works

![gameplay](docs/display.gif)

- **Unit selection.** Drag a box, or click a unit to select it. If shift key is being hold, multiple units can be chosen by clicking.
- **Movement orders.** Right click to send the selected units somewhere.
- **A\* pathfinding.** Units will find the shortest route by A*, however they will go around the obstacles, not through them.
- **Formations.** The chosen units forms a square like formation.
- **Local avoidance.** Units push apart so they do not come on top of each other.
- **Fog of war.** The map starts dark, if a unit is close by, then it is considered as discovered, and it could be either lit or dim, depending on units present.
- **Live debug panel.** FPS, frame time, unit count, per system timings, and buttons to spawn 100/500/1000 units on the fly.
- **Camera.** Pan with WASD, zoom with the mouse wheel, F11 for fullscreen.

## How it's built

### Entity Component System

The project uses a small ECS build around sparse sets. Every aspect of the unit, such as, position, health, selection, movement, pathing, and whatever else that could be added later on, is a seperate component. A unit is basically an ID, and this ID is used for system to find required data.

This approach was used because once, there are a lot of units present, using every components would be wastefull. For instance, the movement code does not need to know the health of a unit, it will only require position and velocity.

The component data sits in a dense array, and another array maps entity ids back to where their data lives in that dense array. So looking up a component is O(1), but iterating over all of them is still just walking through packed memory.

Removing something is also cheap. I swap it with the last item in the dense array and pop it off. No gaps, no shifting half the array around. Entity ids also include a generation value, so an old ID cannot accidentally become valid again just because that slot got reused for a new unit.

### Pathfinding

For the movement, A* with 8 directional movement is used, with its heuristic being octile distance. 

The first version was fine until I started moving units near walls. A\* only sees grid cells. It does not know that the unit sprite or collision body has width.

- **Obstacle inflation.** A path can technically fit through a tight corner on the grid, but then the actual unit can get stucked by the boundary of walls. I fixed that by making a separate pathfinding grid where obstacles are expanded slightly, so an incremented trshold. The player still sees the original walls, but the pathfinder treats them as a bit bigger and gives units more room.
  
- **Nearest open cell retargeting.** This caused another small issue. Clicking close to a wall could mean the clicked cell was inside the inflated obstacle area. In that case the pathfinder thought the destination was blocked and the units just stood there. Now it checks nearby cells and picks the closest valid one, so the order still does something sensible.

### Formations
When a group of units chosen, they form a square resembling grid around the place that is right clicked. Following to that, units are assigned to those slots based on which ones are closest. This is a greedy appoach, so not the best way of doing this. However for this stage it gives acceptable results. 

Once the slots are assigned, there is nothing special left to do. Each uni gets its own targer slot and uses the same A\* pathfinding as normal movement.

### Fog of war

Each cell is either hidden, explored, or currently visible.

This is a very standard approach throuh games, the area arount units is bright, while places you have already been stay visible but darker. Meanwhile, unexplored part are dark.

There are no enemies in the sandbox yet, so the difference between explored and visible is mostly there for the visuals right now. But the data is already there for hiding enemy units later.

## Performance

The simulation is mostly CPU limited. Rendering is just simple 2D drawing, so the GPU is not used very much. The expensive part is updating lots of units, finding paths, checking separation, and doing that for every frame.

I added a benchmark mode. It runs the systems without input or rendering, uses fixed unit counts, and averages the timings across a few hundred frames.

The first bad result was unit separation. The original version checked every unit against every other unit. That is O(n²), which gets ugly very quickly. Double the number of units and you are doing about four times as many checks.

At 4000 units, separation alone was taking around 7 ms. With a 16.6 ms frame budget for 60 FPS, that was already too much time for one system before pathfinding or anything else had even been added.

![fps under load](docs/fps_demo.gif)

To fix that, spatial hash grid is used. A nut can only push another unit that is within about 18 world units of it, so checking thre is no need for checking it against units on the far side of the map. Bucketing units into coarse cells and only comparing against the 9 surrounding buckets turns the all pairs check into "check the handful nearby", which takes it from O(n²) down to roughly O(n).

Separation cost, before and after, measured in a RelWithDebInfo build (single threaded, Intel CPU):

| Units | Before (O(n²)) | After (spatial hash) | Speedup |
|------:|---------------:|---------------------:|--------:|
|   500 |       0.11 ms  |             0.07 ms  |   1.6x  |
|  1000 |       0.46 ms  |             0.21 ms  |   2.2x  |
|  2000 |       1.80 ms  |             0.51 ms  |   3.5x  |
|  4000 |       7.00 ms  |             1.07 ms  |   6.5x  |

The interesting part isn't the raw numbers, it's the shape. Before, separation quadrupled per doubling. After, it roughly doubles, which is the same scaling as the other linear systems. The speedup also grows with load, from 1.6x at 500 units to 6.5x at 4000.

You can reproduce the table with:

```
./build/rts.exe --bench
```

## Building

Needs a C++20 compiler and CMake (3.21 or newer). raylib, Dear ImGui and the rlImGui binding are pulled automatically through CMake's `FetchContent`, so there's nothing to install by hand.

```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
./build/rts.exe
```

Built and tested with MinGW-w64 GCC on Windows. The first configure takes a minute or two while it clones and compiles the dependencies. After that, builds are fast.

## Controls

| Action | Input |
|---|---|
| Select (box) | left click drag |
| Select (single) | left click |
| Add to selection | shift + click or drag |
| Move order | right click |
| Pan camera | WASD |
| Zoom | mouse wheel |
| Fullscreen | F11 |



## Upcoming Improvements
I am planning to implement a few more details, such as;
- **Multithreaded pathfinding.** A\* requests are independent, so they will parallelize well across a job system.
- **Deterministic, fixed timestep simulation.** Needed for replays and the foundation of any networked RTS
