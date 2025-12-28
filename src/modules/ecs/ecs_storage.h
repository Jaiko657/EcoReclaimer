#pragma once
#include "modules/ecs/ecs.h"
#include "modules/ecs/ecs_resource.h"

void cmp_add_storage(ecs_entity_t e, int capacity);

bool ecs_storage_get(ecs_entity_t e, int out_counts[RESOURCE_TYPE_COUNT], int* out_capacity);
bool ecs_storage_add_resource(ecs_entity_t e, resource_type_t type, int count);

ecs_entity_t ecs_storage_find_player(void);
ecs_entity_t ecs_storage_find_tardas(void);
