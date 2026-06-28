# RTS Sandbox

A small real time strategy sandbox written from scratch in C++20, using [raylib](https://www.raylib.com/) for rendering and [Dear ImGui](https://github.com/ocornut/imgui) for the debug tools.

I worked with Raylib before however, I have not worked on an RTS before, so I built one to learn how the core systems actually work, which consists of selecting units, moving them around obstacles, keeping them in formation, and making the whole thing hold up when there are a few thousand units on the map. It is a tech demo, not a game. There is no win condition or enemy AI. The point is the systems underneath.

Everything here is hand written. The entity system, the pathfinding, the formations, the fog of war, none of it comes from an engine or a library. That was the whole idea, to understand the the logic by building it.

![RTS Sandbox](docs/img_game.png)

## What it does

![gameplay](docs/display.gif)

- **Unit selection.** Drag a box to select a group, click to pick one, hold shift to add to the selection.
- **Movement orders.** Right click to send the selected units somewhere.
- **A\* pathfinding.** Units will go around the walls instead of passing through them. 
- **Formations.** Group of chosen units forms a grid shape when they arrive at the destination. They do not cross over each other.
- **Local avoidance.** Units push apart so they don't stack on top of each other.
- **Fog of war.** The map starts dark and units reveal a circle around themselves as they move. Areas you've seen go dim, areas no one can currently see stay hidden.
- **Live debug panel.** FPS, frame time, unit count, per system timings, and buttons to spawn 100 / 500 / 1000 units on the fly.
- **Camera.** Pan with WASD, zoom with the mouse wheel, F11 for fullscreen.

## How it's built

### Entity Component System

The project uses a small ECS built around sparse sets. I did not want units to turn into one huge class that knows about everything: position, health, selection, movement, pathing, whatever else gets added later. So those things are separate components instead. A unit is mostly just an id, and the id is what lets the systems find its data.

That matters once there are a lot of units on screen. The movement code does not need to know a unit's health or whether it is selected. It only needs positions and velocities. Keeping those things packed together means it can go through the data it needs without constantly stepping over unrelated stuff.

At low unit counts, none of this is a big deal. You could probably throw everything into a `Unit` class and never notice. The point of doing it this way was to avoid that becoming a problem once the sandbox had a few thousand units ticking every frame.

The sparse set itself is fairly simple. The component data sits in a dense array, and another array maps entity ids back to where their data lives in that dense array. So looking up a component is O(1), but iterating over all of them is still just walking through packed memory.

Removing something is also cheap. I swap it with the last item in the dense array and pop it off. No gaps, no shifting half the array around. Entity ids also include a generation value, so an old id cannot accidentally become valid again just because that slot got reused for a new unit.

### Pathfinding

Movement uses grid based A\* with 8 directional movement and an octile distance heuristic.

The first version was fine until I started moving units near walls. A\* only sees grid cells. It does not know that the unit sprite or collision body has width.

- **Obstacle inflation.** A path can technically fit through a tight corner on the grid, but then the actual unit catches the wall and gets stuck. I fixed that by making a separate pathfinding grid where obstacles are expanded slightly. The player still sees the original walls, but the pathfinder treats them as a bit wider and gives units more room.
- **Nearest open cell retargeting.** This caused another small issue. Clicking close to a wall could mean the clicked cell was inside the inflated obstacle area. In that case the pathfinder thought the destination was blocked and the units just stood there. Now it checks nearby cells and picks the closest valid one, so the order still does something sensible.

### Formations

For group movement, the game makes a grid of formation slots around the place you clicked. Units are then assigned to those slots based on which ones are closest.

It is greedy, so it is not the best possible assignment in a mathematical sense. I was fine with that. It is quick, and it avoids the obvious nonsense where a unit from the far left runs all the way across the group while another one runs back in the other direction.

Once the slots are assigned, there is nothing special left to do. Each unit gets its own target slot and uses the same A\* pathfinding as normal movement.

### Fog of war

Each cell is either hidden, explored, or currently visible.

Every frame, visible cells are first changed back to explored. Then the units mark the cells around them as visible again. That is what gives the fog its usual RTS behaviour: the area around your units is bright, while places you have already been stay visible but darker.

There are no enemies in the sandbox yet, so the difference between explored and visible is mostly there for the visuals right now. But the data is already there for hiding enemy units later.

## Performance

The simulation is mostly CPU limited. Rendering is just simple 2D drawing, so the GPU is not where the time goes. The expensive part is updating lots of units, finding paths, checking separation, and doing that every frame.

I added a benchmark mode because guessing was getting useless. It runs the systems without input or rendering, uses fixed unit counts, and averages the timings across a few hundred frames.

The first bad result was unit separation. The original version checked every unit against every other unit. That is O(n²), which gets ugly very quickly. Double the number of units and you are doing about four times as many checks.

At 4000 units, separation alone was taking around 7 ms. With a 16.6 ms frame budget for 60 FPS, that was already too much time for one system before pathfinding or anything else had even been added.

![fps under load](docs/fps_demo.gif)

The fix was a spatial hash grid. A unit can only push another unit that's within about 18 world units of it, so checking it against units on the far side of the map is wasted work. Bucketing units into coarse cells and only comparing against the 9 surrounding buckets turns the all pairs check into "check the handful nearby", which takes it from O(n²) down to roughly O(n).

Separation cost, before and after, measured in a RelWithDebInfo build (single threaded, Intel CPU):

| Units | Before (O(n²)) | After (spatial hash) | Speedup |
|------:|---------------:|---------------------:|--------:|
|   500 |       0.11 ms  |             0.07 ms  |   1.6x  |
|  1000 |       0.46 ms  |             0.21 ms  |   2.2x  |
|  2000 |       1.80 ms  |             0.51 ms  |   3.5x  |
|  4000 |       7.00 ms  |             1.07 ms  |   6.5x  |

The interesting part isn't the raw numbers, it's the shape. Before, separation quadrupled per doubling. After, it roughly doubles, which is the same scaling as the other linear systems. The speedup also grows with load, from 1.6x at 500 units to 6.5x at 4000, which is exactly what you'd expect from removing a quadratic term: the bigger the problem, the more it pays off.

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
