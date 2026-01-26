#include "engine/prefab/components/pf_components_engine.h"
#include "engine/ecs/ecs.h"

bool pf_component_pos_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_pos_t* out_pos)
{
    if (!out_pos) return false;
    const tiled_object_t* obj = pf_override_obj(ovr);
    float x = 0.0f, y = 0.0f;
    const char* sx = pf_combined_value(comp, ovr, "x");
    const char* sy = pf_combined_value(comp, ovr, "y");
    bool have = pf_parse_float(sx, &x) && pf_parse_float(sy, &y);
    if (!have) {
        gfx_vec2 p = pf_object_position_default(obj);
        x = p.x;
        y = p.y;
        const char* ox = pf_override_value(comp, ovr, "x");
        const char* oy = pf_override_value(comp, ovr, "y");
        if (ox) pf_parse_float(ox, &x);
        if (oy) pf_parse_float(oy, &y);
    }
    *out_pos = (pf_component_pos_t){ .x = x, .y = y };
    return true;
}

static void pf_component_pos_override(const pf_override_ctx_t* ovr, void* component)
{
    if (!ovr || !ovr->enabled) return;
    const tiled_object_t* obj = ovr->obj;
    pf_component_pos_t* pos = (pf_component_pos_t*)component;
    float x = pf_object_position_default(obj).x;
    float y = pf_object_position_default(obj).y;
    pf_parse_float(pf_object_prop_only(ovr, "POS", "x"), &x);
    pf_parse_float(pf_object_prop_only(ovr, "POS", "y"), &y);
    *pos = (pf_component_pos_t){ .x = x, .y = y };
}

static void pf_component_pos_apply(ecs_entity_t e, const void* component)
{
    const pf_component_pos_t* pos = (const pf_component_pos_t*)component;
    cmp_add_position(e, pos->x, pos->y);
}

const pf_component_ops_t* pf_component_pos_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_POS,
        .component_size = sizeof(pf_component_pos_t),
        .build = (pf_build_fn)pf_component_pos_build,
        .apply = pf_component_pos_apply,
        .override = pf_component_pos_override,
        .override_if_missing = true,
    };
    return &ops;
}
