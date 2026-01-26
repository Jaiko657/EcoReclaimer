#include "game/ecs/ecs_game.h"

void cmp_add_conveyor(ecs_entity_t e, facing_t direction, float speed, bool block_player_input)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_conveyor[i] = (cmp_conveyor_t){
        .direction = direction,
        .speed = speed,
        .block_player_input = block_player_input
    };
    ecs_mask[i] |= CMP_CONVEYOR;
}
