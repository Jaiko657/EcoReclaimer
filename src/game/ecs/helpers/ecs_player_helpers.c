#include "game/ecs/helpers/ecs_player_helpers.h"

#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_engine.h"
#include "shared/components_meta.h"

ecs_entity_t ecs_find_player(void)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (ecs_alive_idx(i) && (ecs_mask[i] & CMP_PLAYER)) {
            return (ecs_entity_t){ .idx = (uint32_t)i, .gen = ecs_gen[i] };
        }
    }
    return ecs_null();
}
