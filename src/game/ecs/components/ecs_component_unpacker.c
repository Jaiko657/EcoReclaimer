#include "game/ecs/ecs_game.h"

void cmp_add_unpacker(ecs_entity_t e)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_unpacker[i] = (cmp_unpacker_t){
        .ready = true,
        .spawned_entity = ecs_null()
    };
    ecs_mask[i] |= CMP_UNPACKER;
}
