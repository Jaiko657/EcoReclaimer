#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_physics_types.h"
#include "engine/asset/tex_handle.h"

// ===== Engine component storage types =====
typedef struct { float x, y; } cmp_position_t;
typedef struct { float x, y; smoothed_facing_t facing; } cmp_velocity_t;

typedef struct {
    bool highlighted;
    colorf highlight_color;
    colorf highlight_base_color;
    bool front;
    int highlight_thickness;
} cmp_sprite_fx_t;

typedef struct {
    tex_handle_t tex;
    rectf src;
    float ox, oy;
    cmp_sprite_fx_t fx;
} cmp_sprite_t;

typedef struct {
    int frame_w;
    int frame_h;

    int anim_count;
    const int* frames_per_anim;       // anim_count entries
    const int* anim_offsets;          // anim_count entries, prefix sum into frames
    const anim_frame_coord_t* frames; // flattened frames table

    int   current_anim;
    int   frame_index;
    float frame_duration;
    float current_time;
} cmp_anim_t;

typedef struct { float hx, hy; } cmp_collider_t;

typedef struct {
    float pad;
    ComponentMask target_mask;
    trigger_match_t match;
} cmp_trigger_t;

typedef struct {
    char text[64];
    float y_offset;
    float linger;
    float timer;
    billboard_state_t state;
} cmp_billboard_t;

// ===== Engine component storage =====
extern cmp_position_t cmp_pos[ECS_MAX_ENTITIES];
extern cmp_velocity_t cmp_vel[ECS_MAX_ENTITIES];
extern cmp_anim_t cmp_anim[ECS_MAX_ENTITIES];
extern cmp_sprite_t cmp_spr[ECS_MAX_ENTITIES];
extern cmp_collider_t cmp_col[ECS_MAX_ENTITIES];
extern cmp_phys_body_t cmp_phys_body[ECS_MAX_ENTITIES];
extern cmp_trigger_t cmp_trigger[ECS_MAX_ENTITIES];
extern cmp_billboard_t cmp_billboard[ECS_MAX_ENTITIES];

void ecs_engine_init(void);
void ecs_engine_shutdown(void);
