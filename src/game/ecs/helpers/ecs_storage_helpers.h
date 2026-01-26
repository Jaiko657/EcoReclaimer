#pragma once

#include "game/ecs/ecs_game.h"

bool ecs_storage_get(ecs_entity_t e, int out_counts[RESOURCE_TYPE_COUNT], int* out_capacity);
bool ecs_storage_add_resource(ecs_entity_t e, resource_type_t type, int count);
bool ecs_storage_take_random(ecs_entity_t e, resource_type_t* out_type);

ecs_entity_t ecs_storage_find_player(void);
ecs_entity_t ecs_storage_find_tardas(void);
