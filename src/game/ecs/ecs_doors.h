#pragma once

#include "engine/ecs/ecs.h"
#include "engine/world/door_tiles.h"

void cmp_add_door(ecs_entity_t e, float prox_radius, int tile_count, const door_tile_xy_t* tile_xy);
