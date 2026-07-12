#pragma once
#include "raylib.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include "world/Grid.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/SeparationSystem.hpp"
#include "systems/FogSystem.hpp"
#include <cstdio>
#include <cstdlib>

namespace rts {

// runs each system frames times on count units and prints the average ms
// no rendering, no input, no camera. just the simulation
inline void run_benchmark(int count,int frames){
    Registry reg;
    Grid grid(63, 63, 32.0f);
    FogMap fog(grid.width, grid.height);

    int worldW = (int)(grid.width * grid.cellSize);
    int worldH = (int)(grid.height * grid.cellSize);

    for (int i = 0; i < count; i++){
        Entity e =reg.create();
        float x= (float)(rand() % worldW);
        float y= (float)(rand() % worldH);
        reg.add<Position>(e, {x,y});
        reg.add<Velocity>(e,{0, 0});
        reg.add<Selectable>(e, {});
        reg.add<Path>(e, {});
        reg.add<Vision>(e, {});
    }

    double sepTotal = 0, moveTotal = 0,fogTotal = 0;
    const float dt = 1.0f / 60.0f;   // pretend a fixed 60fps timestep

    for (int f = 0; f < frames; f++){
        double t0;

        t0 = GetTime();
        update_movement(reg, dt);
        moveTotal += (GetTime() - t0) * 1000.0;

        t0 = GetTime();
        update_separation(reg, grid, dt);
        sepTotal += (GetTime() - t0) * 1000.0;

        t0 = GetTime();
        update_fog(reg, grid, fog);
        fogTotal += (GetTime() - t0) * 1000.0;
    }

    // averages over all frames
    printf("=== %d units, %d frames ===\n", count, frames);
    printf("  movement:   %.4f ms/frame\n", moveTotal / frames);
    printf("  separation: %.4f ms/frame\n", sepTotal / frames);
    printf("  fog:        %.4f ms/frame\n", fogTotal / frames);
    printf("\n");
}


inline void run_benchmark_sweep() {
    printf("\n--- BENCHMARK SWEEP ---\n\n");
    run_benchmark(500, 200);
    run_benchmark(1000, 200);
    run_benchmark(2000, 200);
    run_benchmark(4000, 100);   // fewer frames at 4000 so it doesnt take forever
    printf("--- END ---\n\n");
}

}