#include "engine/core/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "game/ecs/ecs_game.h"
#include <stdio.h>

static bool debug_str_conveyor_rider(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_conveyor_rider_t* r = &cmp_conveyor_rider[idx];
    return snprintf(out, cap, "CONVEYOR_RIDER(active=%d, block=%d, vel=(%.2f,%.2f))",
                    r->active_count, r->block_player_input ? 1 : 0, r->vel_x, r->vel_y) > 0;
}

void debug_str_register_conveyor_rider(void)
{
    debug_str_register(ENUM_CONVEYOR_RIDER, debug_str_conveyor_rider);
}

#endif
