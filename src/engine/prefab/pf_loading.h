#pragma once

#include <stddef.h>
#include "engine/ecs/ecs.h"
#include "engine/tiled/tiled.h"
#include "engine/prefab/prefab.h"

ecs_entity_t pf_spawn_entity(const prefab_t* prefab, const tiled_object_t* obj);
ecs_entity_t pf_spawn_entity_from_path(const char* prefab_path, const tiled_object_t* obj);
size_t pf_spawn_from_map(const world_map_t* map, const char* tmx_path);
