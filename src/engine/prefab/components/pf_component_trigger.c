#include "engine/prefab/pf_components_engine.h"
#include "engine/ecs/ecs.h"

#include <strings.h>

bool pf_component_trigger_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_trigger_t* out_trigger)
{
    if (!out_trigger) return false;
    const tiled_object_t* obj = pf_override_obj(ovr);
    float pad = 0.0f;
    bool have_pad = pf_parse_float(pf_combined_value(comp, ovr, "pad"), &pad);
    if (!have_pad) {
        const char* prox = pf_combined_value(comp, ovr, "proximity_radius");
        if (!prox && obj && obj->proximity_radius > 0) {
            pad = (float)obj->proximity_radius;
        } else {
            pf_parse_float(prox, &pad);
        }
    }

    bool ok = false;
    ComponentMask mask = pf_parse_mask(pf_combined_value(comp, ovr, "target_mask"), &ok);
    const char* match = pf_combined_value(comp, ovr, "match");
    if (!match) match = pf_combined_value(comp, ovr, "target_match");
    trigger_match_t match_mode = TRIGGER_MATCH_ALL;
    if (match && strcasecmp(match, "or") == 0) match_mode = TRIGGER_MATCH_ANY;

    *out_trigger = (pf_component_trigger_t){ pad, mask, match_mode };
    return true;
}

static void pf_component_trigger_apply(ecs_entity_t e, const void* component)
{
    const pf_component_trigger_t* trigger = (const pf_component_trigger_t*)component;
    cmp_add_trigger(e, trigger->pad, trigger->target_mask, trigger->match);
}

const pf_component_ops_t* pf_component_trigger_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_TRIGGER,
        .component_size = sizeof(pf_component_trigger_t),
        .build = (pf_build_fn)pf_component_trigger_build,
        .apply = pf_component_trigger_apply,
    };
    return &ops;
}
