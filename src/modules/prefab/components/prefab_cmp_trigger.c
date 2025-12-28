#include "modules/prefab/prefab_cmp.h"

#include <strings.h>

static trigger_match_t trigger_match_from_string(const char* s, trigger_match_t fallback)
{
    if (!s) return fallback;
    if (strcasecmp(s, "or") == 0 || strcasecmp(s, "any") == 0) return TRIGGER_MATCH_ANY;
    if (strcasecmp(s, "and") == 0 || strcasecmp(s, "all") == 0) return TRIGGER_MATCH_ALL;
    return fallback;
}

bool prefab_cmp_trigger_build(const prefab_component_t* comp, const tiled_object_t* obj, prefab_cmp_trigger_t* out_trigger)
{
    if (!out_trigger) return false;
    float pad = 0.0f;
    bool have_pad = prefab_parse_float(prefab_combined_value(comp, obj, "pad"), &pad);
    if (!have_pad && obj) {
        const char* prox = tiled_object_get_property_value(obj, "proximity_radius");
        if (!prox) prox = prefab_combined_value(comp, obj, "proximity_radius");
        prefab_parse_float(prox, &pad);
    }

    bool ok = false;
    uint32_t mask = prefab_parse_mask(prefab_combined_value(comp, obj, "target_mask"), &ok);
    const char* match = prefab_combined_value(comp, obj, "match");
    if (!match) match = prefab_combined_value(comp, obj, "target_match");
    trigger_match_t match_mode = trigger_match_from_string(match, TRIGGER_MATCH_ALL);
    *out_trigger = (prefab_cmp_trigger_t){ pad, mask, match_mode };
    return true;
}
