#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include <stdio.h>

static bool debug_str_player(ecs_entity_t e, char* out, size_t cap)
{
    (void)e;
    if (!out || cap == 0) return false;
    return snprintf(out, cap, "PLAYER()") > 0;
}

void debug_str_register_player(void)
{
    debug_str_register(ENUM_PLAYER, debug_str_player);
}

#endif
