#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_engine.h"
#include <stdio.h>

static bool debug_str_col(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_collider_t* c = &cmp_col[idx];
    return snprintf(out, cap, "COL(hx=%.2f, hy=%.2f)", c->hx, c->hy) > 0;
}

void debug_str_register_col(void)
{
    debug_str_register(ENUM_COL, debug_str_col);
}

#endif
