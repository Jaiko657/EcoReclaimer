#pragma once

// Game-owned prefab component types and handlers.

#include "engine/prefab/components/pf_component_helpers.h"
#include "engine/prefab/pf_registry.h"
#include "game/ecs/ecs_resource.h"
#include "engine/world/door_tiles.h"
#include "shared/utils/dynarray.h"

typedef struct {
    resource_type_t type;
} pf_component_resource_t;
bool pf_component_resource_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_resource_t* out_resource);

typedef struct {
    int capacity;
    bool has_capacity;
} pf_component_storage_t;
bool pf_component_storage_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_storage_t* out_storage);

typedef struct {
    resource_type_t type;
} pf_component_recycle_bin_t;
bool pf_component_recycle_bin_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_recycle_bin_t* out_recycle_bin);

typedef struct {
    bool has_pickup_distance; float pickup_distance;
    bool has_pickup_radius; float pickup_radius;
    bool has_max_hold_distance; float max_hold_distance;
    bool has_breakoff_distance; float breakoff_distance;
    bool has_follow_gain; float follow_gain;
    bool has_max_speed; float max_speed;
    bool has_damping; float damping;
} pf_component_grav_gun_t;
bool pf_component_grav_gun_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_grav_gun_t* out_grav_gun);

typedef struct {
    facing_t direction;
    float speed;
    bool block_player_input;
} pf_component_conveyor_t;
bool pf_component_conveyor_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_conveyor_t* out_conveyor);

typedef DA(door_tile_xy_t) pf_door_tile_xy_list_t;

typedef struct {
    float prox_radius;
    pf_door_tile_xy_list_t tiles;
} pf_component_door_t;
bool pf_component_door_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_door_t* out_door);
void pf_component_door_free(pf_component_door_t* door);

typedef struct {
    int unused;
} pf_component_unloader_t;
bool pf_component_unloader_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_unloader_t* out_unloader);

const pf_component_ops_t* pf_component_player_ops(void);
const pf_component_ops_t* pf_component_resource_ops(void);
const pf_component_ops_t* pf_component_storage_ops(void);
const pf_component_ops_t* pf_component_recycle_bin_ops(void);
const pf_component_ops_t* pf_component_liftable_ops(void);
const pf_component_ops_t* pf_component_conveyor_ops(void);
const pf_component_ops_t* pf_component_door_ops(void);
const pf_component_ops_t* pf_component_grav_gun_ops(void);
const pf_component_ops_t* pf_component_gun_charger_ops(void);
const pf_component_ops_t* pf_component_unpacker_ops(void);
const pf_component_ops_t* pf_component_unloader_ops(void);
