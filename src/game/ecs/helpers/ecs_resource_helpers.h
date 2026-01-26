#pragma once

#include "game/ecs/ecs_game.h"

bool resource_type_from_string(const char* s, resource_type_t* out_type);
const char* resource_type_to_string(resource_type_t type);

resource_type_t cmp_resource_type_from_index(int idx);
bool cmp_resource_get_type(ecs_entity_t e, resource_type_t* out_type);

void ecs_register_resource_component_hooks(void);
