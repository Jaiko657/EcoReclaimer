#pragma once

// Engine-owned prefab component types and handlers.

#include "engine/prefab/components/pf_component_helpers.h"
#include "engine/prefab/registry/pf_registry.h"

typedef struct {
    float x, y;
} pf_component_pos_t;
bool pf_component_pos_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_pos_t* out_pos);

typedef struct {
    float x, y;
    facing_t dir;
} pf_component_vel_t;
bool pf_component_vel_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_vel_t* out_vel);

typedef struct {
    PhysicsType type;
    float mass;
    unsigned int category_bits;
    unsigned int mask_bits;
} pf_component_phys_body_t;
bool pf_component_phys_body_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_phys_body_t* out_body);

typedef struct {
    const char* path;
    gfx_rect src;
    float ox, oy;
} pf_component_spr_t;
bool pf_component_spr_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_spr_t* out_spr);

typedef struct {
    int frame_w;
    int frame_h;
    float fps;
    int anim_count;
    int frame_buffer_width;
    int frame_buffer_height;
    int* frames_per_anim;
    anim_frame_coord_t* frames;
} pf_component_anim_t;

static inline bool pf_component_anim_frame_coord_valid(const pf_component_anim_t* anim, int seq, int frame)
{
    if (!anim) return false;
    if (anim->frame_buffer_height <= 0 || anim->frame_buffer_width <= 0) return false;
    if (!anim->frames) return false;
    if (seq < 0 || seq >= anim->frame_buffer_height) return false;
    if (frame < 0 || frame >= anim->frame_buffer_width) return false;
    return true;
}

static inline anim_frame_coord_t* pf_component_anim_frame_coord_mut(pf_component_anim_t* anim, int seq, int frame)
{
    if (!pf_component_anim_frame_coord_valid(anim, seq, frame)) return NULL;
    int offset = seq * anim->frame_buffer_width + frame;
    return &anim->frames[offset];
}

static inline const anim_frame_coord_t* pf_component_anim_frame_coord(const pf_component_anim_t* anim, int seq, int frame)
{
    if (!pf_component_anim_frame_coord_valid(anim, seq, frame)) return NULL;
    int offset = seq * anim->frame_buffer_width + frame;
    return &anim->frames[offset];
}

bool pf_component_anim_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_anim_t* out_anim);
void pf_component_anim_free(pf_component_anim_t* anim);

typedef struct { float hx, hy; } pf_component_col_t;
bool pf_component_col_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_col_t* out_col);

typedef struct { float pad; ComponentMask target_mask; trigger_match_t match; } pf_component_trigger_t;
bool pf_component_trigger_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_trigger_t* out_trigger);

typedef struct {
    const char* text;
    float y_offset;
    float linger;
    billboard_state_t state;
} pf_component_billboard_t;
bool pf_component_billboard_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_billboard_t* out_billboard);

const pf_component_ops_t* pf_component_pos_ops(void);
const pf_component_ops_t* pf_component_vel_ops(void);
const pf_component_ops_t* pf_component_phys_body_ops(void);
const pf_component_ops_t* pf_component_spr_ops(void);
const pf_component_ops_t* pf_component_anim_ops(void);
const pf_component_ops_t* pf_component_col_ops(void);
const pf_component_ops_t* pf_component_trigger_ops(void);
const pf_component_ops_t* pf_component_billboard_ops(void);
