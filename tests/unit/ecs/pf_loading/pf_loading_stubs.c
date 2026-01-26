#include "pf_loading_stubs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game/ecs/ecs_game.h"
#include "engine/ecs/ecs_render.h"
#include "engine/core/logger/logger.h"
#include "engine/prefab/components/pf_components_engine.h"
#include "engine/prefab/components/pf_component_helpers.h"
#include "engine/prefab/registry/pf_registry.h"
#include "engine/tiled/tiled.h"

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

int g_cmp_add_position_calls = 0;
int g_cmp_add_size_calls = 0;
float g_cmp_add_size_last_hx = 0.0f;
float g_cmp_add_size_last_hy = 0.0f;
int g_prefab_load_calls = 0;
char g_prefab_load_path[256];
bool g_prefab_load_result = true;
int g_log_warn_calls = 0;

void pf_loading_stub_reset(void)
{
    memset(ecs_mask, 0, sizeof(ecs_mask));
    memset(ecs_gen, 0, sizeof(ecs_gen));
    g_player = ecs_null();
    g_cmp_add_position_calls = 0;
    g_cmp_add_size_calls = 0;
    g_cmp_add_size_last_hx = 0.0f;
    g_cmp_add_size_last_hy = 0.0f;
    g_prefab_load_calls = 0;
    g_prefab_load_path[0] = '\0';
    g_prefab_load_result = true;
    g_log_warn_calls = 0;
}

void pf_loading_stub_set_player(ecs_entity_t e)
{
    g_player = e;
}

ecs_entity_t ecs_create(void)
{
    ecs_gen[0] = 1;
    return (ecs_entity_t){0, 1};
}

int ent_index_checked(ecs_entity_t e)
{
    return (e.idx < ECS_MAX_ENTITIES && ecs_gen[e.idx] == e.gen && e.gen != 0) ? (int)e.idx : -1;
}

ecs_entity_t ecs_find_player(void)
{
    return g_player;
}

void cmp_add_position(ecs_entity_t e, float x, float y)
{
    int idx = ent_index_checked(e);
    if (idx < 0) return;
    g_cmp_add_position_calls++;
    cmp_pos[idx] = (cmp_position_t){ x, y };
    ecs_mask[idx] |= CMP_POS;
}

void cmp_add_velocity(ecs_entity_t e, float x, float y, facing_t direction)
{
    (void)e; (void)x; (void)y; (void)direction;
}

void cmp_add_phys_body(ecs_entity_t e, PhysicsType type, float mass)
{
    (void)e; (void)type; (void)mass;
}

void cmp_add_size(ecs_entity_t e, float hx, float hy)
{
    (void)e;
    g_cmp_add_size_calls++;
    g_cmp_add_size_last_hx = hx;
    g_cmp_add_size_last_hy = hy;
}

void cmp_add_sprite_path(ecs_entity_t e, const char* path, rectf src, float ox, float oy)
{
    (void)e; (void)path; (void)src; (void)ox; (void)oy;
}

void cmp_add_anim(ecs_entity_t e, int frame_w, int frame_h, int anim_count, const int* frames_per_anim, const anim_frame_coord_t* frames, int frame_buffer_width, float fps)
{
    (void)e; (void)frame_w; (void)frame_h; (void)anim_count;
    (void)frames_per_anim; (void)frames; (void)fps; (void)frame_buffer_width;
}

void cmp_add_player(ecs_entity_t e)
{
    (void)e;
}

void cmp_add_storage(ecs_entity_t e, int capacity)
{
    (void)e; (void)capacity;
}

void cmp_add_recycle_bin(ecs_entity_t e, resource_type_t type)
{
    (void)e; (void)type;
}

void cmp_add_liftable(ecs_entity_t e)
{
    int idx = ent_index_checked(e);
    if (idx < 0) return;
    ecs_mask[idx] |= CMP_LIFTABLE;
}

void cmp_add_grav_gun(ecs_entity_t e)
{
    int idx = ent_index_checked(e);
    if (idx < 0) return;
    ecs_mask[idx] |= CMP_GRAV_GUN;
}

void cmp_add_gun_charger(ecs_entity_t e)
{
    int idx = ent_index_checked(e);
    if (idx < 0) return;
    ecs_mask[idx] |= CMP_GUN_CHARGER;
}

void cmp_add_trigger(ecs_entity_t e, float pad, ComponentMask target_mask, trigger_match_t match)
{
    (void)e; (void)pad; (void)target_mask; (void)match;
}

void cmp_add_conveyor(ecs_entity_t e, facing_t dir, float speed, bool block_player_input)
{
    (void)e; (void)dir; (void)speed; (void)block_player_input;
}

void cmp_add_billboard(ecs_entity_t e, const char* text, float y_off, float linger, billboard_state_t state)
{
    (void)e; (void)text; (void)y_off; (void)linger; (void)state;
}

void cmp_add_door(ecs_entity_t e, float prox_radius, int tile_count, const door_tile_xy_t* tile_xy)
{
    (void)e; (void)prox_radius; (void)tile_count; (void)tile_xy;
}

const char* tiled_object_get_property_value(const tiled_object_t* obj, const char* name)
{
    if (!obj || !name) return NULL;
    for (size_t i = 0; i < obj->property_count; ++i) {
        if (obj->properties[i].name && strcmp(obj->properties[i].name, name) == 0) {
            return obj->properties[i].value;
        }
    }
    return NULL;
}

bool prefab_load(const char* path, prefab_t* out_prefab)
{
    g_prefab_load_calls++;
    snprintf(g_prefab_load_path, sizeof(g_prefab_load_path), "%s", path ? path : "");
    if (!g_prefab_load_result) return false;
    if (out_prefab) {
        memset(out_prefab, 0, sizeof(*out_prefab));
    }
    return true;
}

void prefab_free(prefab_t* prefab)
{
    (void)prefab;
}

bool pf_component_pos_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_pos_t* out_pos)
{
    (void)comp; (void)ovr; (void)out_pos;
    return false;
}

bool pf_component_vel_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_vel_t* out_vel)
{
    (void)comp; (void)ovr; (void)out_vel;
    return false;
}

bool pf_component_phys_body_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_phys_body_t* out_body)
{
    (void)comp; (void)ovr; (void)out_body;
    return false;
}

bool pf_component_spr_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_spr_t* out_spr)
{
    (void)comp; (void)ovr; (void)out_spr;
    return false;
}

bool pf_component_anim_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_anim_t* out_anim)
{
    (void)comp; (void)ovr; (void)out_anim;
    return false;
}

void pf_component_anim_free(pf_component_anim_t* anim)
{
    (void)anim;
}

bool pf_component_col_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_col_t* out_col)
{
    (void)comp; (void)ovr; (void)out_col;
    return false;
}

bool pf_component_grav_gun_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_grav_gun_t* out_grav_gun)
{
    (void)comp; (void)ovr; (void)out_grav_gun;
    return false;
}

bool pf_component_storage_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_storage_t* out_storage)
{
    (void)comp; (void)ovr; (void)out_storage;
    return false;
}

bool pf_component_recycle_bin_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_recycle_bin_t* out_recycle_bin)
{
    (void)comp; (void)ovr; (void)out_recycle_bin;
    return false;
}

bool pf_component_trigger_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_trigger_t* out_trigger)
{
    (void)comp; (void)ovr; (void)out_trigger;
    return false;
}

bool pf_component_conveyor_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_conveyor_t* out_conveyor)
{
    (void)comp; (void)ovr; (void)out_conveyor;
    return false;
}

bool pf_component_billboard_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_billboard_t* out_billboard)
{
    (void)comp; (void)ovr; (void)out_billboard;
    return false;
}

bool pf_component_door_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_door_t* out_door)
{
    (void)comp; (void)ovr; (void)out_door;
    return false;
}

void pf_component_door_free(pf_component_door_t* out_door)
{
    (void)out_door;
}

static void stub_pos_override(const pf_override_ctx_t* ovr, void* component)
{
    const tiled_object_t* obj = pf_override_obj(ovr);
    if (!obj) return;
    pf_component_pos_t* pos = (pf_component_pos_t*)component;
    float x = pf_object_position_default(obj).x;
    float y = pf_object_position_default(obj).y;
    pf_parse_float(pf_object_prop_only(ovr, "POS", "x"), &x);
    pf_parse_float(pf_object_prop_only(ovr, "POS", "y"), &y);
    *pos = (pf_component_pos_t){ x, y };
}

static void stub_col_override(const pf_override_ctx_t* ovr, void* component)
{
    const tiled_object_t* obj = pf_override_obj(ovr);
    if (!obj || !component) return;
    pf_component_col_t* col = (pf_component_col_t*)component;
    float hx = 0.0f, hy = 0.0f;
    const bool have_hx = pf_parse_float(pf_object_prop_only(ovr, "COL", "hx"), &hx);
    const bool have_hy = pf_parse_float(pf_object_prop_only(ovr, "COL", "hy"), &hy);
    if (!have_hx && obj->w > 0.0f) hx = obj->w * 0.5f;
    if (!have_hy && obj->h > 0.0f) hy = obj->h * 0.5f;
    *col = (pf_component_col_t){ hx, hy };
}

static void stub_pos_apply(ecs_entity_t e, const void* component)
{
    const pf_component_pos_t* pos = (const pf_component_pos_t*)component;
    cmp_add_position(e, pos->x, pos->y);
}

static void stub_col_apply(ecs_entity_t e, const void* component)
{
    const pf_component_col_t* col = (const pf_component_col_t*)component;
    cmp_add_size(e, col->hx, col->hy);
}

static void stub_noop_apply(ecs_entity_t e, const void* component)
{
    (void)e;
    (void)component;
}

void pf_register_engine_components(void)
{
    pf_register_set(&(pf_component_ops_t){
        .id = ENUM_POS,
        .component_size = sizeof(pf_component_pos_t),
        .build = (pf_build_fn)pf_component_pos_build,
        .apply = stub_pos_apply,
        .override = stub_pos_override,
        .override_if_missing = true,
    });

    pf_register_set(&(pf_component_ops_t){
        .id = ENUM_COL,
        .component_size = sizeof(pf_component_col_t),
        .build = (pf_build_fn)pf_component_col_build,
        .apply = stub_col_apply,
        .override = stub_col_override,
        .override_if_missing = false,
    });

    pf_register_set(&(pf_component_ops_t){
        .id = ENUM_SPR,
        .component_size = sizeof(pf_component_spr_t),
        .build = (pf_build_fn)pf_component_spr_build,
        .apply = stub_noop_apply,
    });
}

void pf_register_game_components(void)
{
}

bool log_would_log(log_level_t lvl)
{
    (void)lvl;
    return true;
}

void log_msg(log_level_t lvl, const log_cat_t* cat, const char* fmt, ...)
{
    (void)cat; (void)fmt;
    if (lvl == LOG_LVL_WARN) g_log_warn_calls++;
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
void ecs_anim_shutdown_allocator(void) {}
