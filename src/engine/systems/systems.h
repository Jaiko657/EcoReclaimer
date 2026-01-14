#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "engine/input/input.h"

// Phase pipeline (conceptual):
// Input -> Intent -> Physics -> Post-Sim -> Pre-Render -> Render -> GC
//
// Mapped to enums:
// PHASE_INPUT: Input
// PHASE_SIM_PRE: Intent (pre-sim state setup)
// PHASE_PHYSICS: Physics integration and motion
// PHASE_SIM_POST: Post-sim views/derived state
// PHASE_PRE_RENDER: Pre-render (UI, camera, animation prep, non-render updates)
// PHASE_RENDER: Render + GC (world draw, UI draw, asset GC)
// PHASE_DEBUG: Debug-only systems
// PHASE_RENDER: render-phase systems
//
// Phases shared between ECS and other systems.
typedef enum {
    PHASE_INPUT = 0,
    PHASE_SIM_PRE,
    PHASE_PHYSICS,
    PHASE_SIM_POST,
    PHASE_DEBUG,
    PHASE_PRE_RENDER,
    PHASE_RENDER,
    PHASE_COUNT
} systems_phase_t;

// Signature for any registered system.
typedef void (*systems_fn)(float dt, const input_t* in);

// Registry API.
void systems_init(void);
void systems_register(systems_phase_t phase, int order, systems_fn fn, const char* name);
void systems_run_phase(systems_phase_t phase, float dt, const input_t* in);

typedef struct {
    const char* name;
    int order;
    systems_fn fn;
} systems_info_t;

bool systems_get_phase_systems(systems_phase_t phase, const systems_info_t** out_list, size_t* out_count);

void systems_tick(float dt, const input_t* in);
void systems_present(float frame_dt);
