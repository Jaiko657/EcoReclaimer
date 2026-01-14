#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "engine/core/engine_types.h"
#include "shared/components_meta.h"
#include "engine/ecs/ecs_physics_types.h"
// TODO: ANIMATION SYSTEM IMPROVMENTS: locality bad, and fixed size bugs
#define MAX_ANIMS  16

typedef struct {
    int count[100]; // if I use this more than 100 times I'm an idiot
    int num;
} ecs_count_result_t; //For debug data passing

// ====== Public constants / types ======
#define ECS_MAX_ENTITIES 1024

typedef struct {
    uint32_t idx;
    uint32_t gen;     // generation (0 == dead)
} ecs_entity_t;

static inline ecs_entity_t ecs_null(void){ return (ecs_entity_t){UINT32_MAX,0}; }

// facing / animation helpers
typedef enum {
    DIR_NORTH = 0,
    DIR_NORTHEAST,
    DIR_EAST,
    DIR_SOUTHEAST,
    DIR_SOUTH,
    DIR_SOUTHWEST,
    DIR_WEST,
    DIR_NORTHWEST,
} facing_t;

typedef struct {
    facing_t rawDir;
    facing_t facingDir;
    facing_t candidateDir;
    float candidateTime;
} smoothed_facing_t;

typedef struct {
    uint8_t col;
    uint8_t row;
} anim_frame_coord_t;

typedef enum { BILLBOARD_INACTIVE = 0, BILLBOARD_ACTIVE = 1 } billboard_state_t;

typedef enum {
    GRAV_GUN_STATE_FREE = 0,
    GRAV_GUN_STATE_HELD = 1
} grav_gun_state_t;

typedef enum {
    TRIGGER_MATCH_ALL = 0,
    TRIGGER_MATCH_ANY = 1
} trigger_match_t;

// ====== Lifecycle / config ======
void ecs_init(void);
void ecs_shutdown(void);
bool ecs_get_position(ecs_entity_t e, v2f* out_pos);

// ====== Entity / components ======
ecs_entity_t ecs_create(void);
void         ecs_destroy(ecs_entity_t e);
void         ecs_mark_destroy(ecs_entity_t e);
void         ecs_cleanup_marked(void);
void         ecs_destroy_marked(void);

void cmp_add_position (ecs_entity_t e, float x, float y);
void cmp_add_velocity(ecs_entity_t e, float x, float y, facing_t direction);
void cmp_add_anim(
    ecs_entity_t e,
    int frame_w,
    int frame_h,
    int anim_count,
    const int* frames_per_anim,
    const anim_frame_coord_t* frames,
    int frame_buffer_width,
    float fps);
void cmp_add_player   (ecs_entity_t e);
void cmp_add_trigger  (ecs_entity_t e, float pad, ComponentMask target_mask, trigger_match_t match);
void cmp_add_conveyor (ecs_entity_t e, facing_t direction, float speed, bool block_player_input);
void cmp_add_billboard(ecs_entity_t e, const char* text, float y_off, float linger, billboard_state_t state);
void cmp_add_size     (ecs_entity_t e, float hx, float hy); // AABB half-extents
void cmp_add_liftable (ecs_entity_t e);
void cmp_add_grav_gun (ecs_entity_t e);
void cmp_add_gun_charger(ecs_entity_t e);
void cmp_add_unpacker(ecs_entity_t e);
void cmp_add_unloader(ecs_entity_t e, ecs_entity_t unpacker_handle);
void cmp_add_phys_body(ecs_entity_t e, PhysicsType type, float mass, unsigned int category_bits, unsigned int mask_bits);

// ====== HUD helpers ======
ecs_count_result_t ecs_count_entities(const ComponentMask* masks, int num_masks);

// toast
bool        ecs_toast_is_active(void);
const char* ecs_toast_get_text(void);
