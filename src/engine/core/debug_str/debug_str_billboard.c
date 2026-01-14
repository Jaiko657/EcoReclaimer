#include "engine/core/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_engine.h"
#include <stdio.h>

static const char* billboard_state_short(billboard_state_t s)
{
    switch (s) {
        case BILLBOARD_INACTIVE: return "OFF";
        case BILLBOARD_ACTIVE:   return "ON";
        default:                 return "?";
    }
}

static bool debug_str_billboard(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_billboard_t* b = &cmp_billboard[idx];
    return snprintf(out, cap, "BILLBOARD(text=\"%.24s\", y_off=%.1f, linger=%.1f, state=%s)",
                    b->text, b->y_offset, b->linger, billboard_state_short(b->state)) > 0;
}

void debug_str_register_billboard(void)
{
    debug_str_register(ENUM_BILLBOARD, debug_str_billboard);
}

#endif
