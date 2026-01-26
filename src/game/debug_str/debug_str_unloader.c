#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "game/ecs/ecs_game.h"
#include <stdio.h>

static bool debug_str_unloader(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_unloader_t* u = &cmp_unloader[idx];
    return snprintf(out, cap, "UNLOADER(unpacker=%u)", u->unpacker_handle.idx) > 0;
}

void debug_str_register_unloader(void)
{
    debug_str_register(ENUM_UNLOADER, debug_str_unloader);
}

#endif
