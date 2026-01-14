#include "engine/prefab/pf_components_engine.h"
#include "engine/ecs/ecs.h"

#include <strings.h>

static billboard_state_t parse_billboard_state(const char* s, billboard_state_t fallback)
{
    if (!s) return fallback;
    if (strcasecmp(s, "active") == 0) return BILLBOARD_ACTIVE;
    if (strcasecmp(s, "inactive") == 0) return BILLBOARD_INACTIVE;
    if (strcasecmp(s, "hidden") == 0) return BILLBOARD_INACTIVE;
    return fallback;
}

bool pf_component_billboard_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_billboard_t* out_billboard)
{
    if (!out_billboard) return false;
    const char* text = pf_combined_value(comp, ovr, "text");
    float y_off = 0.0f;
    float linger = 0.0f;
    billboard_state_t state = BILLBOARD_ACTIVE;
    pf_parse_float(pf_combined_value(comp, ovr, "y_offset"), &y_off);
    pf_parse_float(pf_combined_value(comp, ovr, "linger"), &linger);
    state = parse_billboard_state(pf_combined_value(comp, ovr, "state"), state);

    *out_billboard = (pf_component_billboard_t){
        .text = text,
        .y_offset = y_off,
        .linger = linger,
        .state = state,
    };
    return true;
}

static void pf_component_billboard_apply(ecs_entity_t e, const void* component)
{
    const pf_component_billboard_t* bb = (const pf_component_billboard_t*)component;
    cmp_add_billboard(e, bb->text, bb->y_offset, bb->linger, bb->state);
}

const pf_component_ops_t* pf_component_billboard_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_BILLBOARD,
        .component_size = sizeof(pf_component_billboard_t),
        .build = (pf_build_fn)pf_component_billboard_build,
        .apply = pf_component_billboard_apply,
    };
    return &ops;
}
