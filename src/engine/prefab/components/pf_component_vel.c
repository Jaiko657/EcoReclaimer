#include "engine/prefab/components/pf_components_engine.h"
#include "engine/ecs/ecs.h"

bool pf_component_vel_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_vel_t* out_vel)
{
    if (!out_vel) return false;
    float vx = 0.0f, vy = 0.0f;
    pf_parse_float(pf_combined_value(comp, ovr, "x"), &vx);
    pf_parse_float(pf_combined_value(comp, ovr, "y"), &vy);
    facing_t dir = pf_parse_facing(pf_combined_value(comp, ovr, "dir"), DIR_SOUTH);
    *out_vel = (pf_component_vel_t){ .x = vx, .y = vy, .dir = dir };
    return true;
}

static void pf_component_vel_apply(ecs_entity_t e, const void* component)
{
    const pf_component_vel_t* vel = (const pf_component_vel_t*)component;
    cmp_add_velocity(e, vel->x, vel->y, vel->dir);
}

const pf_component_ops_t* pf_component_vel_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_VEL,
        .component_size = sizeof(pf_component_vel_t),
        .build = (pf_build_fn)pf_component_vel_build,
        .apply = pf_component_vel_apply,
    };
    return &ops;
}
