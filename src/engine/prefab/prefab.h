#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "engine/ecs/ecs.h"

typedef struct {
    char* name;
    char* value;
} prefab_kv_t;

typedef struct prefab_component_t {
    ComponentEnum id;
    char* type_name;
    char* xml;
    prefab_kv_t* props;
    size_t prop_count;
    bool override_after_spawn;
} prefab_component_t;

typedef struct prefab_t {
    char* name;
    prefab_component_t* components;
    size_t component_count;
} prefab_t;

bool prefab_load(const char* path, prefab_t* out_prefab);
void prefab_free(prefab_t* prefab);
