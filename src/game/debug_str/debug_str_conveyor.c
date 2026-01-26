#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "game/ecs/ecs_game.h"
#include <stdio.h>

static bool debug_str_conveyor(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_conveyor_t* c = &cmp_conveyor[idx];
    return snprintf(out, cap, "CONVEYOR(dir=%d, speed=%.2f, block=%d)",
                    c->direction, c->speed, c->block_player_input ? 1 : 0) > 0;
}

void debug_str_register_conveyor(void)
{
    debug_str_register(ENUM_CONVEYOR, debug_str_conveyor);
}

#endif
