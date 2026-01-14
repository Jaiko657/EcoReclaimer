#include "engine/core/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "game/ecs/ecs_game.h"
#include <stdio.h>

static const char* grav_gun_state_short(grav_gun_state_t s)
{
    switch (s) {
        case GRAV_GUN_STATE_FREE: return "FREE";
        case GRAV_GUN_STATE_HELD: return "HELD";
        default:                  return "?";
    }
}

static bool debug_str_liftable(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_liftable_t* l = &cmp_liftable[idx];
    return snprintf(out, cap, "LIFTABLE(state=%s, holder=%u, drop=%d)",
                    grav_gun_state_short(l->state), l->holder.idx, l->just_dropped ? 1 : 0) > 0;
}

void debug_str_register_liftable(void)
{
    debug_str_register(ENUM_LIFTABLE, debug_str_liftable);
}

#endif
