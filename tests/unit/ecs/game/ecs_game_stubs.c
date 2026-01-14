#include "ecs_game_stubs.h"
#include "engine/world/world_door_handle.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "game/ecs/ecs_game.h"
#include "game/ecs/ecs_proximity.h"
#include "engine/systems/systems.h"
#include "engine/core/logger.h"
#include "engine/tiled/tiled.h"
#include "engine/world/world.h"
#include "engine/world/world_door.h"
#include "engine/prefab/pf_spawning.h"
#include "engine/world/world_renderer.h"

void sys_storage_deposit_adapt(float dt, const input_t* in);
void sys_doors_tick_adapt(float dt, const input_t* in);

ComponentMask   ecs_mask[ECS_MAX_ENTITIES];
uint32_t        ecs_gen[ECS_MAX_ENTITIES];
uint32_t        ecs_next_gen[ECS_MAX_ENTITIES];
cmp_position_t  cmp_pos[ECS_MAX_ENTITIES];
cmp_velocity_t  cmp_vel[ECS_MAX_ENTITIES];
cmp_anim_t      cmp_anim[ECS_MAX_ENTITIES];
cmp_sprite_t    cmp_spr[ECS_MAX_ENTITIES];
cmp_collider_t  cmp_col[ECS_MAX_ENTITIES];
cmp_phys_body_t cmp_phys_body[ECS_MAX_ENTITIES];

static ecs_entity_t g_player = {0, 0};
const world_map_t* g_world_tiled_map = NULL;
int g_pf_spawn_calls = 0;
char g_pf_spawn_last_path[256];
int g_ecs_register_system_calls = 0;
int g_world_door_apply_calls = 0;
int g_world_door_anim_calls = 0;
int g_world_door_primary_duration = 0;
float g_world_door_last_time = 0.0f;
bool g_world_door_last_forward = false;
int g_ui_toast_calls = 0;
char g_ui_toast_last[128];
systems_fn g_ecs_sys_storage = NULL;
systems_fn g_ecs_sys_doors_tick = NULL;

static ecs_prox_view_t g_prox_enter_views[8];
static size_t g_prox_enter_count = 0;
static ecs_prox_view_t g_prox_stay_views[8];
static size_t g_prox_stay_count = 0;

void ecs_game_stub_reset(void)
{
    memset(ecs_mask, 0, sizeof(ecs_mask));
    memset(ecs_gen, 0, sizeof(ecs_gen));
    memset(cmp_pos, 0, sizeof(cmp_pos));
    memset(cmp_col, 0, sizeof(cmp_col));
    g_player = (ecs_entity_t){0, 0};
    g_world_tiled_map = NULL;
    g_pf_spawn_calls = 0;
    g_pf_spawn_last_path[0] = '\0';
    g_ecs_register_system_calls = 0;
    g_world_door_apply_calls = 0;
    g_world_door_anim_calls = 0;
    g_world_door_primary_duration = 0;
    g_world_door_last_time = 0.0f;
    g_world_door_last_forward = false;
    g_ui_toast_calls = 0;
    g_ui_toast_last[0] = '\0';
    g_ecs_sys_storage = NULL;
    g_ecs_sys_doors_tick = NULL;
    memset(g_prox_enter_views, 0, sizeof(g_prox_enter_views));
    memset(g_prox_stay_views, 0, sizeof(g_prox_stay_views));
    g_prox_enter_count = 0;
    g_prox_stay_count = 0;
}

void ecs_game_stub_set_player(ecs_entity_t e)
{
    g_player = e;
}

void ecs_game_stub_set_prox_enter(const ecs_prox_view_t* views, size_t count)
{
    g_prox_enter_count = (count > 8) ? 8 : count;
    if (views && g_prox_enter_count) {
        memcpy(g_prox_enter_views, views, g_prox_enter_count * sizeof(*views));
    }
}

void ecs_game_stub_set_prox_stay(const ecs_prox_view_t* views, size_t count)
{
    g_prox_stay_count = (count > 8) ? 8 : count;
    if (views && g_prox_stay_count) {
        memcpy(g_prox_stay_views, views, g_prox_stay_count * sizeof(*views));
    }
}

bool ecs_alive_idx(int i)
{
    return ecs_gen[i] != 0;
}

int ent_index_checked(ecs_entity_t e)
{
    return (e.idx < ECS_MAX_ENTITIES && ecs_gen[e.idx] == e.gen && e.gen != 0) ? (int)e.idx : -1;
}

ecs_entity_t find_player_handle(void)
{
    return g_player;
}

ecs_entity_t ecs_find_player(void)
{
    return g_player;
}

ecs_entity_t handle_from_index(int i)
{
    if (i < 0 || i >= ECS_MAX_ENTITIES) return ecs_null();
    if (ecs_gen[i] == 0) return ecs_null();
    return (ecs_entity_t){ (uint32_t)i, ecs_gen[i] };
}

void ecs_register_component_destroy_hook(ComponentEnum comp, ecs_component_hook_fn fn)
{
    (void)comp;
    (void)fn;
}

void ecs_destroy(ecs_entity_t e)
{
    int idx = ent_index_checked(e);
    if (idx < 0) return;
    ecs_gen[idx] = 0;
    ecs_mask[idx] = 0;
}

void ecs_phys_body_destroy_for_entity(int idx)
{
    if (idx < 0 || idx >= ECS_MAX_ENTITIES) return;
    cmp_phys_body[idx].created = false;
}

ecs_prox_iter_t ecs_prox_enter_begin(void) { return (ecs_prox_iter_t){ .i = 0 }; }

bool ecs_prox_enter_next(ecs_prox_iter_t* it, ecs_prox_view_t* out)
{
    if (!it || !out) return false;
    if ((size_t)it->i >= g_prox_enter_count) return false;
    *out = g_prox_enter_views[it->i++];
    return true;
}

ecs_prox_iter_t ecs_prox_stay_begin(void) { return (ecs_prox_iter_t){ .i = 0 }; }

bool ecs_prox_stay_next(ecs_prox_iter_t* it, ecs_prox_view_t* out)
{
    if (!it || !out) return false;
    if ((size_t)it->i >= g_prox_stay_count) return false;
    *out = g_prox_stay_views[it->i++];
    return true;
}

void ui_toast(float secs, const char* fmt, ...)
{
    (void)secs;
    g_ui_toast_calls++;
    if (!fmt) return;
    va_list args;
    va_start(args, fmt);
    vsnprintf(g_ui_toast_last, sizeof(g_ui_toast_last), fmt, args);
    va_end(args);
}

bool log_would_log(log_level_t lvl)
{
    (void)lvl;
    return true;
}

void log_msg(log_level_t lvl, const log_cat_t* cat, const char* fmt, ...)
{
    (void)lvl; (void)cat; (void)fmt;
}

const world_map_t* world_get_map(void)
{
    return g_world_tiled_map;
}

bool world_has_map(void)
{
    return g_world_tiled_map != NULL;
}

size_t pf_spawn_from_map(const world_map_t* map, const char* tmx_path)
{
    g_pf_spawn_calls++;
    if (tmx_path) {
        snprintf(g_pf_spawn_last_path, sizeof(g_pf_spawn_last_path), "%s", tmx_path);
    }
    return 0;
}

void world_door_apply_state(world_door_handle_t handle, float t_ms, bool play_forward)
{
    (void)handle;
    g_world_door_last_time = t_ms;
    g_world_door_last_forward = play_forward;
    g_world_door_apply_calls++;
}

int world_door_primary_animation_duration(world_door_handle_t handle)
{
    (void)handle;
    g_world_door_anim_calls++;
    return g_world_door_primary_duration;
}

void systems_register(systems_phase_t phase, int order, systems_fn fn, const char* name)
{
    (void)phase; (void)order; (void)fn; (void)name;
    g_ecs_register_system_calls++;
    if (!name) return;
    if (strcmp(name, "storage_deposit") == 0) {
        g_ecs_sys_storage = fn;
    } else if (strcmp(name, "doors_tick") == 0) {
        g_ecs_sys_doors_tick = fn;
    }
}

void systems_registration_init(void)
{
    systems_register(PHASE_SIM_POST, 120, sys_storage_deposit_adapt, "storage_deposit");
    systems_register(PHASE_SIM_POST, 400, sys_doors_tick_adapt, "doors_tick");
}
