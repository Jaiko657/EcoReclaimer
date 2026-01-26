#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "game/ecs/ecs_game.h"
#include <stdio.h>

static const char* door_state_short(int s)
{
    switch (s) {
        case DOOR_CLOSED:  return "CLOSED";
        case DOOR_OPENING: return "OPENING";
        case DOOR_OPEN:    return "OPEN";
        case DOOR_CLOSING: return "CLOSING";
        default:           return "?";
    }
}

static bool debug_str_door(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_door_t* d = &cmp_door[idx];
    return snprintf(out, cap, "DOOR(state=%s, prox=%.1f, tiles=%d, intent=%d)",
                    door_state_short(d->state), d->prox_radius, d->tile_count,
                    d->intent_open ? 1 : 0) > 0;
}

void debug_str_register_door(void)
{
    debug_str_register(ENUM_DOOR, debug_str_door);
}

#endif
