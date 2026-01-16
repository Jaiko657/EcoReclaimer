//==== FROM ecs_core.c ====
#include "game/ecs/ecs_game.h"
#include "game/ecs/ecs_resource.h"
#include "engine/core/logger.h"
#include "game/prefab/pf_register_game.h"
#include "shared/utils/build_config.h"
#include <math.h>
#include <string.h>

// =============== ECS Storage =============
cmp_player_t    cmp_player[ECS_MAX_ENTITIES];
cmp_conveyor_t  cmp_conveyor[ECS_MAX_ENTITIES];
cmp_conveyor_rider_t cmp_conveyor_rider[ECS_MAX_ENTITIES];
cmp_liftable_t  cmp_liftable[ECS_MAX_ENTITIES];
cmp_grav_gun_t  cmp_grav_gun[ECS_MAX_ENTITIES];
cmp_gun_charger_t cmp_gun_charger[ECS_MAX_ENTITIES];
cmp_door_t      cmp_door[ECS_MAX_ENTITIES];
cmp_unloader_t  cmp_unloader[ECS_MAX_ENTITIES];
cmp_unpacker_t  cmp_unpacker[ECS_MAX_ENTITIES];

void ecs_register_grav_gun_component_hooks(void);
void ecs_register_liftable_component_hooks(void);
void ecs_register_resource_component_hooks(void);
void ecs_door_on_destroy(int idx);

static ecs_entity_t find_player_handle(void)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (ecs_alive_idx(i) && (ecs_mask[i] & CMP_PLAYER)) {
            return (ecs_entity_t){ (uint32_t)i, ecs_gen[i] };
        }
    }
    return ecs_null();
}

ecs_entity_t ecs_find_player(void)
{
    return find_player_handle();
}

bool ecs_get_player_position(float* out_x, float* out_y)
{
    ecs_entity_t player = find_player_handle();
    int idx = ent_index_checked(player);
    if (idx < 0 || !(ecs_mask[idx] & CMP_POS)) return false;

    if (out_x) *out_x = cmp_pos[idx].x;
    if (out_y) *out_y = cmp_pos[idx].y;
    return true;
}

void game_init(void)
{
    pf_register_game_components();
    ecs_register_grav_gun_component_hooks();
    ecs_register_liftable_component_hooks();
    ecs_register_resource_component_hooks();
    ecs_register_component_destroy_hook(ENUM_DOOR, ecs_door_on_destroy);
#if DEBUG_BUILD
    debug_str_game_register_all();
#endif
}

void ecs_game_shutdown(void)
{
}

void cmp_add_player(ecs_entity_t e)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_player[i] = (cmp_player_t){
        .held_gun = ecs_null(),
        .held_liftable = ecs_null()
    };
    ecs_mask[i] |= CMP_PLAYER;
}

void cmp_add_conveyor(ecs_entity_t e, facing_t direction, float speed, bool block_player_input)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_conveyor[i] = (cmp_conveyor_t){
        .direction = direction,
        .speed = speed,
        .block_player_input = block_player_input
    };
    ecs_mask[i] |= CMP_CONVEYOR;
}

void cmp_add_liftable(ecs_entity_t e)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_liftable[i] = (cmp_liftable_t){
        .state              = GRAV_GUN_STATE_FREE,
        .holder             = ecs_null(),
        .pickup_distance    = 0.0f,
        .pickup_radius      = 0.0f,
        .max_hold_distance  = 0.0f,
        .breakoff_distance  = 0.0f,
        .follow_gain        = 0.0f,
        .max_speed          = 0.0f,
        .damping            = 0.0f,
        .hold_vel_x         = 0.0f,
        .hold_vel_y         = 0.0f,
        .grab_offset_x      = 0.0f,
        .grab_offset_y      = 0.0f,
        .just_dropped       = false,
        .recycle_active     = false,
        .recycle_target_y   = 0.0f
    };
    ecs_mask[i] |= CMP_LIFTABLE;
}

void cmp_add_grav_gun(ecs_entity_t e)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_grav_gun[i] = (cmp_grav_gun_t){
        .held = false,
        .holder = ecs_null(),
        .charge = 1.0f,
        .max_charge = 1.0f,
        .drain_rate = 0.15f,
        .regen_rate = 0.25f,
        .eject_timer = 0.0f,
        .toast_pending = false
    };
    ecs_mask[i] |= CMP_GRAV_GUN;
}

void cmp_add_gun_charger(ecs_entity_t e)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_gun_charger[i] = (cmp_gun_charger_t){
        .stored_gun = ecs_null(),
        .flash_timer = 0.0f
    };
    ecs_mask[i] |= CMP_GUN_CHARGER;
}

void cmp_add_unpacker(ecs_entity_t e)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_unpacker[i] = (cmp_unpacker_t){
        .ready = true,
        .spawned_entity = ecs_null()
    };
    ecs_mask[i] |= CMP_UNPACKER;
}

void cmp_add_unloader(ecs_entity_t e, ecs_entity_t unpacker_handle)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_unloader[i] = (cmp_unloader_t){
        .unpacker_handle = unpacker_handle
    };
    ecs_mask[i] |= CMP_UNLOADER;
}



