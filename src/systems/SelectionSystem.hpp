#pragma once
#include "raylib.h"
#include "ecs/Registry.hpp"
#include "ecs/Components.hpp"
#include <cmath> 

namespace rts{

// holds the drag state between frames, main owns one of these and passes
struct SelectionState {
    bool dragging = false;
    Vector2 startWorld = {0, 0};// where we pressed, in world coords
    Vector2 endWorld   = {0, 0}; // where the mouse is, ""
};

inline void update_selection(Registry& reg, SelectionState& sel,const Camera2D& cam){
    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), cam);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        sel.dragging = true;
        sel.startWorld = mouseWorld;
        sel.endWorld = mouseWorld;
    }

    if (sel.dragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        sel.endWorld = mouseWorld;
    }

    if (sel.dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        sel.dragging = false;

        // holding shift means add to the current selection instead of replacing it, if pressed without it release the chosen ones
        bool additive = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        float minX = fminf(sel.startWorld.x, sel.endWorld.x);
        float maxX = fmaxf(sel.startWorld.x, sel.endWorld.x);
        float minY = fminf(sel.startWorld.y, sel.endWorld.y);
        float maxY = fmaxf(sel.startWorld.y, sel.endWorld.y);

        float boxW = maxX - minX;
        float boxH = maxY - minY;
        bool isClick = (boxW < 5.0f && boxH < 5.0f);

        if (isClick){
            Entity best = NULL_ENTITY;
            float bestDist =9999999.0f;

            // find the closest unit to the click, but only clear everyone
            reg.view<Position, Selectable>([&](Entity e, Position& p, Selectable& s){
                if (!additive) 
                    s.selected = false;
                
                    float dx = p.x - mouseWorld.x;
                float dy = p.y - mouseWorld.y;
                float dist = dx*dx + dy*dy;
                if (dist < 144.0f && dist < bestDist){
                    bestDist = dist;
                    best = e;
                }
            });

            if(best != NULL_ENTITY){
                reg.get<Selectable>(best).selected = true;
            }
        }
        else {
            reg.view<Position, Selectable>([&](Entity, Position& p, Selectable& s){
                bool inside = (p.x >= minX && p.x <= maxX && p.y >= minY && p.y <= maxY);
                if (additive)
                    s.selected = s.selected || inside; // add, never unselect
                else
                    s.selected = inside; // fresh selection
            });
        }
    }
}
inline void draw_selection_box(const SelectionState& sel){
    if (!sel.dragging) return;

    float x = fminf(sel.startWorld.x,sel.endWorld.x);
    float y = fminf(sel.startWorld.y,sel.endWorld.y);
    float w = fabsf(sel.endWorld.x -sel.startWorld.x);
    float h = fabsf(sel.endWorld.y -sel.startWorld.y);

    // translucent fill + solid outline
    DrawRectangleRec({x, y, w, h}, Fade(GREEN, 0.15f));
    DrawRectangleLinesEx({x, y, w, h},2.0f, GREEN);
}

}