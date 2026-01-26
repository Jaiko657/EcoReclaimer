#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include <stdio.h>

static bool debug_str_recycle_bin(ecs_entity_t e, char* out, size_t cap)
{
    (void)e;
    if (!out || cap == 0) return false;
    return snprintf(out, cap, "RECYCLE_BIN()") > 0;
}

void debug_str_register_recycle_bin(void)
{
    debug_str_register(ENUM_RECYCLE_BIN, debug_str_recycle_bin);
}

#endif
