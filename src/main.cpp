#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include "world/Grid.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/SelectionSystem.hpp"
#include "systems/FormationSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/SeparationSystem.hpp"
#include "systems/FogSystem.hpp"
#include "debug/Profiler.hpp"
#include "debug/DebugUI.hpp"
#include "bench/Benchmark.hpp"
#include <cstdlib>   // rand
#include <string>    // for the --bench arg check

using namespace rts;

static void spawn_units(Registry& reg, const Grid& grid, int count) {
    int worldW = (int)(grid.width *grid.cellSize);
    int worldH = (int)(grid.height *grid.cellSize);
    for (int i = 0; i < count; i++){
        float x, y;
        do{
            x =(float)(rand() % worldW);
            y =(float)(rand() % worldH);
        }while (grid.is_blocked(grid.world_to_cell_x(x), grid.world_to_cell_y(y)));

        Entity e = reg.create();
        reg.add<Position>(e,{x, y});
        reg.add<Velocity>(e, {0, 0});
        reg.add<Selectable>(e, {});
        reg.add<Path>(e,{});
        reg.add<Vision>(e, {});
    }
}

int main(int argc, char** argv){
    // run "./build/rts.exe --bench" to do the benchmark sweep and exit, no game.
    // tiny window because GetTime() needs raylib initialized
    if (argc > 1 && std::string(argv[1]) == "--bench") {
        InitWindow(1, 1, "bench");
        run_benchmark_sweep();
        CloseWindow();
        return 0;
    }

    InitWindow(1280, 720, "RTS Sandbox");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    rlImGuiSetup(true);

    Registry reg;

    Grid grid(63, 63, 32.0f);
    for (int cy = 10; cy < 40; cy++) grid.set_blocked(25, cy, true);
    for (int cx = 25; cx < 50; cx++) grid.set_blocked(cx, 40, true);
    Grid navGrid = inflate_obstacles(grid, 1);

    FogMap fog(grid.width, grid.height);

    int unitCount = 200;
    spawn_units(reg, grid, unitCount);

    Camera2D cam = {};
    cam.target = {1000, 1000};
    cam.offset = {GetScreenWidth()/2.0f, GetScreenHeight()/2.0f};
    cam.zoom = 0.5f;

    SelectionState sel;
    Profiler prof;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();
        cam.offset = {GetScreenWidth()/2.0f, GetScreenHeight()/2.0f};

        float panSpeed = 500.0f * dt / cam.zoom;
        if (IsKeyDown(KEY_W)) cam.target.y -= panSpeed;
        if (IsKeyDown(KEY_S)) cam.target.y += panSpeed;
        if (IsKeyDown(KEY_A)) cam.target.x -= panSpeed;
        if (IsKeyDown(KEY_D)) cam.target.x += panSpeed;

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            cam.zoom += wheel * 0.1f;
            if (cam.zoom < 0.1f) cam.zoom = 0.1f;
            if (cam.zoom > 3.0f) cam.zoom = 3.0f;
        }

        update_selection(reg, sel, cam);

        // time each system: sample the clock before and after, store the gap
        double t0 = prof.now_ms();
        update_formation_orders(reg, cam, navGrid);
        prof.pathfindingMs = prof.now_ms() - t0;

        t0 = prof.now_ms();
        update_movement(reg, dt);
        prof.movementMs = prof.now_ms() - t0;

        t0 = prof.now_ms();
        update_separation(reg, grid, dt);
        prof.separationMs = prof.now_ms() - t0;

        t0 = prof.now_ms();
        update_fog(reg, grid, fog);
        prof.fogMs = prof.now_ms() - t0;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(cam);
        render_grid(grid);
        render_units(reg);
        render_fog(grid, fog);
        draw_selection_box(sel);
        EndMode2D();

        DrawFPS(GetScreenWidth() - 100, 10);

        // debug panel, returns >0 if a spawn button was clicked this frame
        rlImGuiBegin();
        int spawn = draw_debug_ui(reg, prof, unitCount);
        rlImGuiEnd();

        if (spawn > 0) {
            spawn_units(reg, grid, spawn);
            unitCount += spawn;
        }

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}