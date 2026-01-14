#include "engine/core/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "game/ecs/ecs_game.h"
#include <stdio.h>

static bool debug_str_gun_charger(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_gun_charger_t* c = &cmp_gun_charger[idx];
    return snprintf(out, cap, "GUN_CHARGER(stored=%u, flash=%.2f)",
                    c->stored_gun.idx, c->flash_timer) > 0;
}

void debug_str_register_gun_charger(void)
{
    debug_str_register(ENUM_GUN_CHARGER, debug_str_gun_charger);
}

#endif
