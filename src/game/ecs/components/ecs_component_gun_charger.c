#include "game/ecs/ecs_game.h"

void cmp_add_gun_charger(ecs_entity_t e)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_gun_charger[i] = (cmp_gun_charger_t){
        .stored_gun = ecs_null(),
        .flash_timer = 0.0f
    };
    ecs_mask[i] |= CMP_GUN_CHARGER;
}
