#include "game/ecs/ecs_game.h"

void cmp_add_player(ecs_entity_t e)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_player[i] = (cmp_player_t){
        .held_gun = ecs_null(),
        .held_liftable = ecs_null()
    };
    ecs_mask[i] |= CMP_PLAYER;
}
