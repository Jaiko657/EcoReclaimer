#include "engine/core/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_engine.h"
#include <stdio.h>

static bool debug_str_pos(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_position_t* p = &cmp_pos[idx];
    return snprintf(out, cap, "POS(x=%.3f, y=%.3f)", p->x, p->y) > 0;
}

void debug_str_register_pos(void)
{
    debug_str_register(ENUM_POS, debug_str_pos);
}

#endif
