#include "game/ecs/ecs_game.h"

void cmp_add_grav_gun(ecs_entity_t e)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_grav_gun[i] = (cmp_grav_gun_t){
        .held = false,
        .holder = ecs_null(),
        .charge = 1.0f,
        .max_charge = 1.0f,
        .drain_rate = 0.15f,
        .regen_rate = 0.25f,
        .eject_timer = 0.0f,
        .toast_pending = false
    };
    ecs_mask[i] |= CMP_GRAV_GUN;
}
