#include "engine/prefab/pf_components_engine.h"
#include "engine/ecs/ecs_render.h"

#include <stdio.h>

static bool parse_rect(const char* s, rectf* out)
{
    if (!s || !out) return false;
    float x = 0, y = 0, w = 0, h = 0;
    if (sscanf(s, "%f,%f,%f,%f", &x, &y, &w, &h) != 4) return false;
    *out = rectf_xywh(x, y, w, h);
    return true;
}

bool pf_component_spr_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_spr_t* out_spr)
{
    if (!out_spr) return false;

    const char* path = pf_combined_value(comp, ovr, "path");
    if (!path) path = pf_combined_value(comp, ovr, "tex");

    rectf src = rectf_xywh(0, 0, 0, 0);
    parse_rect(pf_combined_value(comp, ovr, "src"), &src);

    float x = src.x, y = src.y, w = src.w, h = src.h;
    pf_parse_float(pf_combined_value(comp, ovr, "src_x"), &x);
    pf_parse_float(pf_combined_value(comp, ovr, "src_y"), &y);
    pf_parse_float(pf_combined_value(comp, ovr, "src_w"), &w);
    pf_parse_float(pf_combined_value(comp, ovr, "src_h"), &h);
    src = rectf_xywh(x, y, w, h);

    float ox = 0.0f, oy = 0.0f;
    pf_parse_float(pf_combined_value(comp, ovr, "ox"), &ox);
    pf_parse_float(pf_combined_value(comp, ovr, "oy"), &oy);

    if (!path) return false;

    *out_spr = (pf_component_spr_t){
        .path = path,
        .src = src,
        .ox = ox,
        .oy = oy,
    };
    return true;
}

static void pf_component_spr_apply(ecs_entity_t e, const void* component)
{
    const pf_component_spr_t* spr = (const pf_component_spr_t*)component;
    cmp_add_sprite_path(e, spr->path, spr->src, spr->ox, spr->oy);
}

const pf_component_ops_t* pf_component_spr_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_SPR,
        .component_size = sizeof(pf_component_spr_t),
        .build = (pf_build_fn)pf_component_spr_build,
        .apply = pf_component_spr_apply,
    };
    return &ops;
}
