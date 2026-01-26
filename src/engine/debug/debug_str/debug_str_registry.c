#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"

static debug_str_fn g_debug_fns[ENUM_COMPONENT_COUNT];

bool debug_str_register(ComponentEnum id, debug_str_fn fn)
{
    if (id < 0 || id >= ENUM_COMPONENT_COUNT) return false;
    g_debug_fns[id] = fn;
    return true;
}

bool debug_str_component(ComponentEnum id, ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0) return false;
    if (id < 0 || id >= ENUM_COMPONENT_COUNT) return false;
    if (!g_debug_fns[id]) return false;
    return g_debug_fns[id](e, out, cap);
}

#endif
