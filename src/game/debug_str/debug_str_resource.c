#include "engine/core/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "game/ecs/ecs_resource.h"
#include <stdio.h>

static bool debug_str_resource(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    resource_type_t type = cmp_resource_type_from_index(idx);
    const char* label = resource_type_to_string(type);
    if (!label) label = "resource";
    return snprintf(out, cap, "RESOURCE(type=%s)", label) > 0;
}

void debug_str_register_resource(void)
{
    debug_str_register(ENUM_RESOURCE, debug_str_resource);
}

#endif
