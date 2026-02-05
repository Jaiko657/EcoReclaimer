#include "debug_hotkeys_stubs.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "engine/asset/asset.h"
#include "engine/runtime/camera.h"
#include "engine/engine/engine_manager/engine_manager.h"
#include "game/ecs/helpers/ecs_storage_helpers.h"
#include "game/ecs/ecs_game.h"
#include "engine/core/logger/logger.h"
#include "engine/renderer/renderer.h"
#include "engine/runtime/toast.h"
#include "engine/world/world_query.h"
#include "engine/input/input.h"

int g_asset_reload_calls = 0;
int g_asset_log_calls = 0;
int g_renderer_ecs_calls = 0;
int g_renderer_phys_calls = 0;
int g_renderer_static_calls = 0;
int g_renderer_triggers_calls = 0;
int g_renderer_fps_calls = 0;
int g_toast_calls = 0;
char g_last_toast[256] = {0};
int g_log_calls = 0;
bool g_engine_reload_world_result = false;
int g_world_tiles_w = 0;
int g_world_tiles_h = 0;
int g_game_storage_counts[RESOURCE_TYPE_COUNT] = {0};
int g_game_storage_capacity = 0;
ComponentMask   ecs_mask[ECS_MAX_ENTITIES];
uint32_t        ecs_gen[ECS_MAX_ENTITIES];
uint32_t        ecs_next_gen[ECS_MAX_ENTITIES];
cmp_position_t  cmp_pos[ECS_MAX_ENTITIES];
cmp_velocity_t  cmp_vel[ECS_MAX_ENTITIES];
cmp_anim_t      cmp_anim[ECS_MAX_ENTITIES];
cmp_sprite_t    cmp_spr[ECS_MAX_ENTITIES];
cmp_collider_t  cmp_col[ECS_MAX_ENTITIES];
cmp_phys_body_t cmp_phys_body[ECS_MAX_ENTITIES];
bool g_ecs_alive[ECS_MAX_ENTITIES];

void debug_hotkeys_stub_reset(void)
{
    g_asset_reload_calls = 0;
    g_asset_log_calls = 0;
    g_renderer_ecs_calls = 0;
    g_renderer_phys_calls = 0;
    g_renderer_static_calls = 0;
    g_renderer_triggers_calls = 0;
    g_renderer_fps_calls = 0;
    g_toast_calls = 0;
    g_last_toast[0] = '\0';
    g_log_calls = 0;
    g_engine_reload_world_result = false;
    g_world_tiles_w = 0;
    g_world_tiles_h = 0;
    for (int i = 0; i < RESOURCE_TYPE_COUNT; ++i) {
        g_game_storage_counts[i] = 0;
    }
    g_game_storage_capacity = 0;
    memset(ecs_mask, 0, sizeof(ecs_mask));
    memset(ecs_gen, 0, sizeof(ecs_gen));
    memset(ecs_next_gen, 0, sizeof(ecs_next_gen));
    memset(cmp_pos, 0, sizeof(cmp_pos));
    memset(cmp_vel, 0, sizeof(cmp_vel));
    memset(cmp_anim, 0, sizeof(cmp_anim));
    memset(cmp_spr, 0, sizeof(cmp_spr));
    memset(cmp_col, 0, sizeof(cmp_col));
    memset(cmp_phys_body, 0, sizeof(cmp_phys_body));
    memset(g_ecs_alive, 0, sizeof(g_ecs_alive));
}

void asset_reload_all(void)
{
    g_asset_reload_calls++;
}

void asset_log_debug(void)
{
    g_asset_log_calls++;
}

bool renderer_toggle_ecs_colliders(void)
{
    g_renderer_ecs_calls++;
    return (g_renderer_ecs_calls % 2) == 1;
}

bool renderer_toggle_phys_colliders(void)
{
    g_renderer_phys_calls++;
    return (g_renderer_phys_calls % 2) == 1;
}

bool renderer_toggle_static_colliders(void)
{
    g_renderer_static_calls++;
    return (g_renderer_static_calls % 2) == 1;
}

bool renderer_toggle_triggers(void)
{
    g_renderer_triggers_calls++;
    return (g_renderer_triggers_calls % 2) == 1;
}

bool renderer_toggle_fps_overlay(void)
{
    g_renderer_fps_calls++;
    return (g_renderer_fps_calls % 2) == 1;
}

void ui_toast(float secs, const char* fmt, ...)
{
    (void)secs;
    g_toast_calls++;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_last_toast, sizeof(g_last_toast), fmt, ap);
    va_end(ap);
}

bool engine_reload_world(void)
{
    return g_engine_reload_world_result;
}

bool world_size_tiles(int* out_w, int* out_h)
{
    if (out_w) *out_w = g_world_tiles_w;
    if (out_h) *out_h = g_world_tiles_h;
    return true;
}

camera_view_t camera_get_view(void)
{
    camera_view_t view = {0};
    view.zoom = 1.0f;
    return view;
}

bool ecs_storage_get(ecs_entity_t e, int out_counts[RESOURCE_TYPE_COUNT], int* out_capacity)
{
    (void)e;
    if (out_counts) {
        for (int type = 0; type < RESOURCE_TYPE_COUNT; ++type) {
            out_counts[type] = g_game_storage_counts[type];
        }
    }
    if (out_capacity) *out_capacity = g_game_storage_capacity;
    return true;
}

bool log_would_log(log_level_t lvl)
{
    (void)lvl;
    return true;
}

void log_msg(log_level_t lvl, const log_cat_t* cat, const char* fmt, ...)
{
    (void)lvl;
    (void)cat;
    (void)fmt;
    g_log_calls++;
}

void log_set_sink(log_sink_fn sink)
{
    (void)sink;
}

void log_set_min_level(log_level_t lvl)
{
    (void)lvl;
}

bool ecs_alive_idx(int i)
{
    if (i < 0 || i >= ECS_MAX_ENTITIES) return false;
    return g_ecs_alive[i];
}

int ent_index_checked(ecs_entity_t e)
{
    if (e.idx < 0 || e.idx >= ECS_MAX_ENTITIES) return -1;
    if (!ecs_alive_idx((int)e.idx)) return -1;
    return (int)e.idx;
}

int ent_index_unchecked(ecs_entity_t e)
{
    if (e.idx < 0 || e.idx >= ECS_MAX_ENTITIES) return -1;
    return (int)e.idx;
}

ecs_entity_t handle_from_index(int i)
{
    if (i < 0 || i >= ECS_MAX_ENTITIES) return ecs_null();
    if (!g_ecs_alive[i]) return ecs_null();
    return (ecs_entity_t){ (uint32_t)i, 1 };
}

void ecs_register_component_destroy_hook(ComponentEnum comp, ecs_component_hook_fn fn)
{
    (void)comp;
    (void)fn;
}

void ecs_register_phys_body_create_hook(ecs_component_hook_fn fn)
{
    (void)fn;
}

void ecs_register_render_component_hooks(void) {}
void ecs_register_physics_component_hooks(void) {}
void ecs_register_grav_gun_component_hooks(void) {}
void ecs_register_liftable_component_hooks(void) {}
void ecs_anim_reset_allocator(void) {}

bool renderer_screen_to_world(float screen_x, float screen_y, float* out_x, float* out_y)
{
    if (out_x) *out_x = screen_x;
    if (out_y) *out_y = screen_y;
    return true;
}

const input_t* input_frame_snapshot(void)
{
    static input_t empty = {0};
    return &empty;
}

bool platform_dir_exists(const char* path)
{
    (void)path;
    return false;
}

bool platform_make_dir(const char* path)
{
    (void)path;
    return false;
}

bool platform_file_exists(const char* path)
{
    (void)path;
    return false;
}

bool platform_take_screenshot(const char* path)
{
    (void)path;
    return false;
}
void ecs_anim_shutdown_allocator(void) {}

// Delegate actual ECS globals/hooks to the real core implementation (linked for debug builds).
