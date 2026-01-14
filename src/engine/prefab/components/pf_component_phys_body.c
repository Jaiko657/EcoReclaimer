#include "engine/prefab/pf_components_engine.h"
#include "engine/ecs/ecs.h"
#include "engine/ecs/ecs_physics_tags.h"

#include <strings.h>
#include <stdlib.h>

static PhysicsType parse_phys_type(const char* s, PhysicsType fallback)
{
    if (!s) return fallback;
    if (strcasecmp(s, "static") == 0) return PHYS_STATIC;
    if (strcasecmp(s, "dynamic") == 0) return PHYS_DYNAMIC;
    if (strcasecmp(s, "kinematic") == 0) return PHYS_KINEMATIC;
    return fallback;
}

static bool parse_bits_u32(const char* s, unsigned int* out_bits)
{
    if (!s || !out_bits) return false;
    if (s[0] == '\0') {
        *out_bits = 0u;
        return true;
    }
    char* end = NULL;
    unsigned long v = strtoul(s, &end, 0);
    if (!end || end == s || *end != '\0') return false;
    *out_bits = (unsigned int)v;
    return true;
}

bool pf_component_phys_body_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_phys_body_t* out_body)
{
    if (!out_body) return false;
    PhysicsType type = parse_phys_type(pf_combined_value(comp, ovr, "type"), PHYS_DYNAMIC);
    float mass = 1.0f;
    pf_parse_float(pf_combined_value(comp, ovr, "mass"), &mass);

    unsigned int category_bits = 0u;
    unsigned int mask_bits = 0u;

    const char* cat_bits_str = pf_combined_value(comp, ovr, "category_bits");
    const char* mask_bits_str = pf_combined_value(comp, ovr, "mask_bits");
    const char* cat_str = pf_combined_value(comp, ovr, "category");
    const char* mask_str = pf_combined_value(comp, ovr, "mask");

    if (cat_bits_str && parse_bits_u32(cat_bits_str, &category_bits)) {
        // numeric override
    } else if (cat_str) {
        category_bits = phys_parse_tag_list(cat_str);
    }

    if (mask_bits_str && parse_bits_u32(mask_bits_str, &mask_bits)) {
        // numeric override
    } else if (mask_str) {
        mask_bits = phys_parse_tag_list(mask_str);
    }

    *out_body = (pf_component_phys_body_t){ type, mass, category_bits, mask_bits };
    return true;
}

static void pf_component_phys_body_apply(ecs_entity_t e, const void* component)
{
    const pf_component_phys_body_t* body = (const pf_component_phys_body_t*)component;
    cmp_add_phys_body(e, body->type, body->mass, body->category_bits, body->mask_bits);
}

const pf_component_ops_t* pf_component_phys_body_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_PHYS_BODY,
        .component_size = sizeof(pf_component_phys_body_t),
        .build = (pf_build_fn)pf_component_phys_body_build,
        .apply = pf_component_phys_body_apply,
    };
    return &ops;
}
