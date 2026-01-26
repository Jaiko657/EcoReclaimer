#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "game/ecs/ecs_game.h"
#include <stdio.h>

static bool debug_str_grav_gun(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_grav_gun_t* g = &cmp_grav_gun[idx];
    return snprintf(out, cap, "GRAV_GUN(held=%d, holder=%u, charge=%.2f/%.2f)",
                    g->held ? 1 : 0, g->holder.idx, g->charge, g->max_charge) > 0;
}

void debug_str_register_grav_gun(void)
{
    debug_str_register(ENUM_GRAV_GUN, debug_str_grav_gun);
}

#endif
