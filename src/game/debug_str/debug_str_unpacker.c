#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "game/ecs/ecs_game.h"
#include <stdio.h>

static bool debug_str_unpacker(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_unpacker_t* u = &cmp_unpacker[idx];
    return snprintf(out, cap, "UNPACKER(ready=%d, spawned=%u)",
                    u->ready ? 1 : 0, u->spawned_entity.idx) > 0;
}

void debug_str_register_unpacker(void)
{
    debug_str_register(ENUM_UNPACKER, debug_str_unpacker);
}

#endif
