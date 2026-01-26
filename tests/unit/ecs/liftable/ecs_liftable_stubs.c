#include "game/ecs/ecs_game.h"
#include "engine/ecs/ecs_proximity.h"
#include "engine/renderer/renderer.h"
#include "engine/world/world.h"
#include "engine/asset/asset.h"
#include "engine/runtime/toast.h"

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

bool ecs_alive_idx(int i)
{
    return ecs_gen[i] != 0;
}

bool ecs_alive_handle(ecs_entity_t e)
{
    return ent_index_checked(e) >= 0;
}

int ent_index_checked(ecs_entity_t e)
{
    return (e.idx < ECS_MAX_ENTITIES && ecs_gen[e.idx] == e.gen && e.gen != 0) ? (int)e.idx : -1;
}

ecs_entity_t find_player_handle(void)
{
    return g_player;
}

void grav_gun_stub_set_player(ecs_entity_t e)
{
    g_player = e;
}

float clampf(float v, float a, float b)
{
    return (v < a) ? a : ((v > b) ? b : v);
}

ecs_entity_t handle_from_index(int i)
{
    return (ecs_entity_t){ (uint32_t)i, ecs_gen[i] };
}

void cmp_add_velocity(ecs_entity_t e, float x, float y, facing_t direction)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    smoothed_facing_t smoothed_dir = {
        .rawDir = direction,
        .facingDir = direction,
        .candidateDir = direction,
        .candidateTime = 0.0f
    };
    cmp_vel[i] = (cmp_velocity_t){ x, y, smoothed_dir };
    ecs_mask[i] |= CMP_VEL;
}

void cmp_add_position(ecs_entity_t e, float x, float y)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_pos[i] = (cmp_position_t){ x, y };
    ecs_mask[i] |= CMP_POS;
}

void cmp_add_sprite_handle(ecs_entity_t e, tex_handle_t h, rectf src, float ox, float oy)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_spr[i] = (cmp_sprite_t){
        .tex = h,
        .src = src,
        .ox = ox,
        .oy = oy
    };
    ecs_mask[i] |= CMP_SPR;
}

bool renderer_screen_to_world(float screen_x, float screen_y, float* out_x, float* out_y)
{
    if (out_x) *out_x = screen_x;
    if (out_y) *out_y = screen_y;
    return true;
}

int world_subtile_size(void)
{
    return 16;
}

bool world_size_px(int* out_w, int* out_h)
{
    if (out_w) *out_w = 1000;
    if (out_h) *out_h = 1000;
    return true;
}

bool world_is_walkable_subtile(int sx, int sy)
{
    (void)sx; (void)sy;
    return true;
}

tex_handle_t asset_acquire_texture(const char* path)
{
    (void)path;
    return (tex_handle_t){ 1, 1 };
}

void asset_release_texture(tex_handle_t h)
{
    (void)h;
}

bool asset_texture_valid(tex_handle_t h)
{
    return h.idx != 0;
}

ecs_prox_iter_t ecs_prox_stay_begin(void)
{
    return (ecs_prox_iter_t){ .i = -1 };
}

bool ecs_prox_stay_next(ecs_prox_iter_t* it, ecs_prox_view_t* out)
{
    (void)it;
    (void)out;
    return false;
}

void ecs_register_component_destroy_hook(ComponentEnum comp, ecs_component_hook_fn fn)
{
    (void)comp;
    (void)fn;
}

void ui_toast_update(float dt)
{
    (void)dt;
}

void ui_toast(float secs, const char* fmt, ...)
{
    (void)secs;
    (void)fmt;
}
