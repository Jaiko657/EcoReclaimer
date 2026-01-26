#include "engine/prefab/components/pf_components_engine.h"
#include "engine/ecs/ecs.h"

bool pf_component_col_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_col_t* out_col)
{
    if (!out_col) return false;
    const tiled_object_t* obj = pf_override_obj(ovr);
    float hx = 0.0f, hy = 0.0f;
    const char* prefab_hx = pf_find_prop(comp, "hx");
    const char* prefab_hy = pf_find_prop(comp, "hy");

    bool have_hx = false;
    bool have_hy = false;

    if (pf_parse_float(prefab_hx, &hx)) have_hx = true;
    if (pf_parse_float(prefab_hy, &hy)) have_hy = true;
    if (!have_hx && obj && pf_parse_float(pf_override_value(comp, ovr, "hx"), &hx)) have_hx = true;
    if (!have_hy && obj && pf_parse_float(pf_override_value(comp, ovr, "hy"), &hy)) have_hy = true;

    if (!have_hx && obj && obj->w > 0.0f) hx = obj->w * 0.5f;
    if (!have_hy && obj && obj->h > 0.0f) hy = obj->h * 0.5f;

    *out_col = (pf_component_col_t){ .hx = hx, .hy = hy };
    return true;
}

static void pf_component_col_override(const pf_override_ctx_t* ovr, void* component)
{
    if (!ovr || !ovr->enabled || !component) return;
    const tiled_object_t* obj = ovr->obj;
    pf_component_col_t* col = (pf_component_col_t*)component;
    float hx = 0.0f, hy = 0.0f;
    const bool have_hx = pf_parse_float(pf_object_prop_only(ovr, "COL", "hx"), &hx);
    const bool have_hy = pf_parse_float(pf_object_prop_only(ovr, "COL", "hy"), &hy);
    if (!have_hx && obj && obj->w > 0.0f) hx = obj->w * 0.5f;
    if (!have_hy && obj && obj->h > 0.0f) hy = obj->h * 0.5f;
    *col = (pf_component_col_t){ .hx = hx, .hy = hy };
}

static void pf_component_col_apply(ecs_entity_t e, const void* component)
{
    const pf_component_col_t* col = (const pf_component_col_t*)component;
    cmp_add_size(e, col->hx, col->hy);
}

const pf_component_ops_t* pf_component_col_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_COL,
        .component_size = sizeof(pf_component_col_t),
        .build = (pf_build_fn)pf_component_col_build,
        .apply = pf_component_col_apply,
        .override = pf_component_col_override,
        .override_if_missing = false,
    };
    return &ops;
}
