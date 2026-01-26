#include "game/ecs/ecs_game.h"

void cmp_add_unloader(ecs_entity_t e, ecs_entity_t unpacker_handle)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_unloader[i] = (cmp_unloader_t){
        .unpacker_handle = unpacker_handle
    };
    ecs_mask[i] |= CMP_UNLOADER;
}
