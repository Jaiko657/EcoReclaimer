#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "engine/ecs/ecs_engine.h"
#include "game/debug_str/debug_str_game.h"

typedef struct render_view_t render_view_t;

// ===== Game component storage types =====
typedef struct {
    ecs_entity_t held_gun;
    ecs_entity_t held_liftable;
} cmp_player_t;

typedef struct {
    facing_t direction;
    float speed;
    bool block_player_input;
} cmp_conveyor_t;

typedef struct {
    int active_count;
    PhysicsType saved_type;
    float saved_mass;
    float saved_inv_mass;
    unsigned int saved_category_bits;
    unsigned int saved_mask_bits;
    bool saved_valid;
    float vel_x;
    float vel_y;
    bool block_player_input;
} cmp_conveyor_rider_t;

typedef struct {
    grav_gun_state_t state;
    ecs_entity_t holder;
    float pickup_distance;
    float pickup_radius;
    float max_hold_distance;
    float breakoff_distance;
    float follow_gain;
    float max_speed;
    float damping;
    float hold_vel_x;
    float hold_vel_y;
    float grab_offset_x;
    float grab_offset_y;
    bool just_dropped;
    bool recycle_active;
    float recycle_target_y;
} cmp_liftable_t;

typedef struct {
    bool held;
    ecs_entity_t holder;
    float charge;
    float max_charge;
    float drain_rate;
    float regen_rate;
    float eject_timer;
    bool toast_pending;
} cmp_grav_gun_t;

typedef struct {
    ecs_entity_t stored_gun;
    float flash_timer;
} cmp_gun_charger_t;

typedef struct {
    int tx;
    int ty;
    int layer_idx;
    int tileset_idx;
    int base_tile_id;
    uint32_t flip_flags;
} door_tile_info_t;

typedef struct {
    float prox_radius;
    door_tile_info_t* tiles;
    int tile_count;
    int primary_anim_total_ms;
    enum { DOOR_CLOSED = 0, DOOR_OPENING, DOOR_OPEN, DOOR_CLOSING } state;
    float anim_time_ms;
    bool intent_open;
} cmp_door_t;

typedef struct {
    ecs_entity_t unpacker_handle;
} cmp_unloader_t;

typedef struct {
    bool ready;
    ecs_entity_t spawned_entity;
} cmp_unpacker_t;

// ===== Game component storage =====
extern cmp_player_t cmp_player[ECS_MAX_ENTITIES];
extern cmp_conveyor_t cmp_conveyor[ECS_MAX_ENTITIES];
extern cmp_conveyor_rider_t cmp_conveyor_rider[ECS_MAX_ENTITIES];
extern cmp_liftable_t cmp_liftable[ECS_MAX_ENTITIES];
extern cmp_grav_gun_t cmp_grav_gun[ECS_MAX_ENTITIES];
extern cmp_gun_charger_t cmp_gun_charger[ECS_MAX_ENTITIES];
extern cmp_door_t cmp_door[ECS_MAX_ENTITIES];
extern cmp_unloader_t cmp_unloader[ECS_MAX_ENTITIES];
extern cmp_unpacker_t cmp_unpacker[ECS_MAX_ENTITIES];

void ecs_game_init(void);
void ecs_game_shutdown(void);

ecs_entity_t ecs_find_player(void);
bool ecs_get_player_position(float* out_x, float* out_y);

bool init_entities(const char* tmx_path);

void ecs_game_render_ui(const render_view_t* view);
