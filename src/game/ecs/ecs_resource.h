#pragma once

#include "engine/ecs/ecs.h"

typedef enum {
    RESOURCE_TYPE_PLASTIC = 0,
    RESOURCE_TYPE_METAL,
    RESOURCE_TYPE_COUNT,
} resource_type_t;

bool resource_type_from_string(const char* s, resource_type_t* out_type);
const char* resource_type_to_string(resource_type_t type);

void cmp_add_resource(ecs_entity_t e, resource_type_t type);
resource_type_t cmp_resource_type_from_index(int idx);
bool cmp_resource_get_type(ecs_entity_t e, resource_type_t* out_type);

void ecs_register_resource_component_hooks(void);
