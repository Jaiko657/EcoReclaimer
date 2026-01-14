#include "engine/core/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_engine.h"
#include <stdio.h>

static bool debug_str_spr(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_sprite_t* s = &cmp_spr[idx];
    return snprintf(out, cap, "SPR(tex=(%u,%u), src=[%.1f,%.1f,%.1f,%.1f])",
                    s->tex.idx, s->tex.gen, s->src.x, s->src.y, s->src.w, s->src.h) > 0;
}

void debug_str_register_spr(void)
{
    debug_str_register(ENUM_SPR, debug_str_spr);
}

#endif
