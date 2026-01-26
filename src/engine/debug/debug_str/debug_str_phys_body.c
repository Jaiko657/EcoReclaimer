#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_engine.h"
#include <stdio.h>

static const char* phys_type_short(PhysicsType t)
{
    switch (t) {
        case PHYS_NONE:      return "NONE";
        case PHYS_DYNAMIC:   return "DYN";
        case PHYS_KINEMATIC: return "KIN";
        case PHYS_STATIC:    return "STA";
        default:             return "?";
    }
}

static bool debug_str_phys_body(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_phys_body_t* b = &cmp_phys_body[idx];
    return snprintf(out, cap,
                    "PHYS_BODY(type=%s, mass=%.3f, cat=0x%X, mask=0x%X, def_type=%s, def_cat=0x%X, def_mask=0x%X, created=%d)",
                    phys_type_short(b->type), b->mass, b->category_bits, b->mask_bits,
                    phys_type_short(b->default_type),
                    b->default_category_bits, b->default_mask_bits,
                    b->created ? 1 : 0) > 0;
}

void debug_str_register_phys_body(void)
{
    debug_str_register(ENUM_PHYS_BODY, debug_str_phys_body);
}

#endif
