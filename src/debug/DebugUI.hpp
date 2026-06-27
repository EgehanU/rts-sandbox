#pragma once
#include "imgui.h"
#include "raylib.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include "debug/Profiler.hpp"

namespace rts {

// draws the debug panel each frame. returns how many units the user asked to
// spawn via the buttons this frame 
inline int draw_debug_ui(Registry& reg, const Profiler& prof, int unitCount) {
    int spawnRequest = 0;

    ImGui::Begin("Debug");

    ImGui::Text("FPS: %d", GetFPS());
    ImGui::Text("Frame: %.2f ms", GetFrameTime() * 1000.0f);
    ImGui::Text("Units: %d", unitCount);

    int selected = 0;
    reg.view<Selectable, Position>([&](Entity, Selectable& s, Position&){
        if (s.selected) selected++;
    });
    ImGui::Text("Selected: %d", selected);

    ImGui::Separator();

    ImGui::Text("Pathfinding: %.3f ms",prof.pathfindingMs);
    ImGui::Text("Movement:    %.3f ms", prof.movementMs);
    ImGui::Text("Separation:  %.3f ms", prof.separationMs);
    ImGui::Text("Fog:         %.3f ms", prof.fogMs);

    ImGui::Separator();

    if (ImGui::Button("Spawn 100"))  
        spawnRequest = 100;
    ImGui::SameLine();
    if (ImGui::Button("Spawn 500"))  
        spawnRequest = 500;
    ImGui::SameLine();
    if (ImGui::Button("Spawn 1000")) 
        spawnRequest = 1000;

    ImGui::End();

    return spawnRequest;
}

}