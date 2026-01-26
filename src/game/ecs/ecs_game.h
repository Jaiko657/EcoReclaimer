#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "engine/ecs/ecs_engine.h"
#include "engine/world/door_tiles.h"

typedef enum {
    GRAV_GUN_STATE_FREE = 0,
    GRAV_GUN_STATE_HELD = 1
} grav_gun_state_t;

typedef enum {
    RESOURCE_TYPE_PLASTIC = 0,
    RESOURCE_TYPE_METAL,
    RESOURCE_TYPE_COUNT,
} resource_type_t;

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

typedef struct {
    int counts[RESOURCE_TYPE_COUNT];
    int capacity;
} cmp_storage_t;

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
extern resource_type_t cmp_resource_type[ECS_MAX_ENTITIES];
extern cmp_storage_t cmp_storage[ECS_MAX_ENTITIES];

void ecs_game_init(void);
void ecs_game_shutdown(void);

void cmp_add_player(ecs_entity_t e);
void cmp_add_conveyor(ecs_entity_t e, facing_t direction, float speed, bool block_player_input);
void cmp_add_liftable(ecs_entity_t e);
void cmp_add_grav_gun(ecs_entity_t e);
void cmp_add_gun_charger(ecs_entity_t e);
void cmp_add_unpacker(ecs_entity_t e);
void cmp_add_unloader(ecs_entity_t e, ecs_entity_t unpacker_handle);
void cmp_add_door(ecs_entity_t e, float prox_radius, int tile_count, const door_tile_xy_t* tile_xy);
void cmp_add_resource(ecs_entity_t e, resource_type_t type);
void cmp_add_storage(ecs_entity_t e, int capacity);
void cmp_add_recycle_bin(ecs_entity_t e, resource_type_t type);
