//==== FROM ecs_core.c ====
#include "engine/ecs/ecs_engine.h"
#include "engine/core/logger.h"
#include "engine/core/debug_str/debug_str_engine.h"
#include <math.h>
#include <string.h>

// =============== ECS Storage =============
cmp_position_t  cmp_pos[ECS_MAX_ENTITIES];
cmp_velocity_t  cmp_vel[ECS_MAX_ENTITIES];
cmp_anim_t      cmp_anim[ECS_MAX_ENTITIES];
cmp_sprite_t    cmp_spr[ECS_MAX_ENTITIES];
cmp_collider_t  cmp_col[ECS_MAX_ENTITIES];
cmp_phys_body_t cmp_phys_body[ECS_MAX_ENTITIES];
cmp_trigger_t   cmp_trigger[ECS_MAX_ENTITIES];
cmp_billboard_t cmp_billboard[ECS_MAX_ENTITIES];

ecs_component_hook_fn phys_body_create_hook = NULL;

static void try_create_phys_body(int i){
    const ComponentMask req = (CMP_POS | CMP_COL | CMP_PHYS_BODY);
    if ((ecs_mask[i] & req) != req) return;
    if (cmp_phys_body[i].created) return;
    if (phys_body_create_hook) {
        phys_body_create_hook(i);
    }
}

bool ecs_get_position(ecs_entity_t e, v2f* out_pos){
    int idx = ent_index_checked(e);
    if (idx < 0 || !(ecs_mask[idx] & CMP_POS)) return false;
    if (out_pos) {
        *out_pos = v2f_make(cmp_pos[idx].x, cmp_pos[idx].y);
    }
    return true;
}

void cmp_add_position(ecs_entity_t e, float x, float y)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    ecs_mask[i] |= CMP_POS;
    cmp_pos[i] = (cmp_position_t){ x, y };
    try_create_phys_body(i);
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

void cmp_add_trigger(ecs_entity_t e, float pad, ComponentMask target_mask, trigger_match_t match)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_trigger[i] = (cmp_trigger_t){ pad, target_mask, match };
    ecs_mask[i] |= CMP_TRIGGER;
}

void cmp_add_billboard(ecs_entity_t e, const char* text, float y_off, float linger, billboard_state_t state)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    if ((ecs_mask[i] & (CMP_TRIGGER)) != (CMP_TRIGGER)) {
        LOGC(LOGCAT_ECS, LOG_LVL_WARN, "Billboard added to entity without trigger. ENTITY: %i, %i", e.idx, e.gen);
    }
    strncpy(cmp_billboard[i].text, text, sizeof(cmp_billboard[i].text) - 1);
    cmp_billboard[i].text[sizeof(cmp_billboard[i].text) - 1] = '\0';
    cmp_billboard[i].y_offset = y_off;
    cmp_billboard[i].linger   = linger;
    cmp_billboard[i].timer    = 0.0f;
    cmp_billboard[i].state    = state;
    ecs_mask[i] |= CMP_BILLBOARD;
}

void cmp_add_size(ecs_entity_t e, float hx, float hy)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_col[i] = (cmp_collider_t){ hx, hy };
    ecs_mask[i] |= CMP_COL;
    try_create_phys_body(i);
}

void cmp_add_phys_body(ecs_entity_t e, PhysicsType type, float mass, unsigned int category_bits, unsigned int mask_bits)
{
    int i = ent_index_checked(e);
    if (i < 0) return;

    float inv_mass = (mass != 0.0f) ? (1.0f / mass) : 0.0f;
    cmp_phys_body[i] = (cmp_phys_body_t){
        .type = type,
        .mass = mass,
        .inv_mass = inv_mass,
        .category_bits = category_bits,
        .mask_bits = mask_bits,
        .default_category_bits = category_bits,
        .default_mask_bits = mask_bits,
        .default_type = type,
        .created = false
    };
    ecs_mask[i] |= CMP_PHYS_BODY;
    try_create_phys_body(i);
}

void ecs_register_render_component_hooks(void);
void ecs_register_physics_component_hooks(void);
void ecs_anim_reset_allocator(void);
void ecs_anim_shutdown_allocator(void);

void ecs_engine_init(void)
{
    ecs_register_render_component_hooks();
    ecs_register_physics_component_hooks();
    ecs_anim_reset_allocator();
    debug_str_engine_register_all();
}

void ecs_engine_shutdown(void)
{
    ecs_anim_shutdown_allocator();
}

