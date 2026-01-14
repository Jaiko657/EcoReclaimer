#include "game/prefab/pf_components_game.h"
#include "engine/ecs/ecs_core.h"
#include "game/ecs/ecs_game.h"

bool pf_component_grav_gun_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_grav_gun_t* out_grav_gun)
{
    if (!out_grav_gun) return false;

    float tmp = 0.0f;
    *out_grav_gun = (pf_component_grav_gun_t){0};

    if (pf_parse_float(pf_combined_value(comp, ovr, "pickup_distance"), &tmp)) { out_grav_gun->has_pickup_distance = true; out_grav_gun->pickup_distance = tmp; }
    if (pf_parse_float(pf_combined_value(comp, ovr, "pickup_radius"), &tmp)) { out_grav_gun->has_pickup_radius = true; out_grav_gun->pickup_radius = tmp; }
    if (pf_parse_float(pf_combined_value(comp, ovr, "max_hold_distance"), &tmp)) { out_grav_gun->has_max_hold_distance = true; out_grav_gun->max_hold_distance = tmp; }
    if (pf_parse_float(pf_combined_value(comp, ovr, "breakoff_distance"), &tmp)) { out_grav_gun->has_breakoff_distance = true; out_grav_gun->breakoff_distance = tmp; }
    if (pf_parse_float(pf_combined_value(comp, ovr, "follow_gain"), &tmp)) { out_grav_gun->has_follow_gain = true; out_grav_gun->follow_gain = tmp; }
    if (pf_parse_float(pf_combined_value(comp, ovr, "max_speed"), &tmp)) { out_grav_gun->has_max_speed = true; out_grav_gun->max_speed = tmp; }
    if (pf_parse_float(pf_combined_value(comp, ovr, "damping"), &tmp)) { out_grav_gun->has_damping = true; out_grav_gun->damping = tmp; }

    return true;
}

static void pf_component_liftable_apply(ecs_entity_t e, const void* component)
{
    const pf_component_grav_gun_t* liftable = (const pf_component_grav_gun_t*)component;
    cmp_add_liftable(e);
    int idx = ent_index_checked(e);
    if (idx >= 0 && (ecs_mask[idx] & CMP_LIFTABLE)) {
        if (liftable->has_pickup_distance) cmp_liftable[idx].pickup_distance = liftable->pickup_distance;
        if (liftable->has_pickup_radius) cmp_liftable[idx].pickup_radius = liftable->pickup_radius;
        if (liftable->has_max_hold_distance) cmp_liftable[idx].max_hold_distance = liftable->max_hold_distance;
        if (liftable->has_breakoff_distance) cmp_liftable[idx].breakoff_distance = liftable->breakoff_distance;
        if (liftable->has_follow_gain) cmp_liftable[idx].follow_gain = liftable->follow_gain;
        if (liftable->has_max_speed) cmp_liftable[idx].max_speed = liftable->max_speed;
        if (liftable->has_damping) cmp_liftable[idx].damping = liftable->damping;
    }
}

const pf_component_ops_t* pf_component_liftable_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_LIFTABLE,
        .component_size = sizeof(pf_component_grav_gun_t),
        .build = (pf_build_fn)pf_component_grav_gun_build,
        .apply = pf_component_liftable_apply,
    };
    return &ops;
}
