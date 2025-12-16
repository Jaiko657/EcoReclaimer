#pragma once
#include "modules/ecs/ecs.h"
#include "modules/ecs/ecs_resource.h"

bool init_entities(const char* tmx_path);

// Component adders (game-specific)
void cmp_add_storage(ecs_entity_t e, int capacity);

// Game HUD helpers
bool game_get_tardas_storage(int out_counts[RESOURCE_TYPE_COUNT], int* out_capacity);
bool game_get_grav_gun_charge(float* out_charge, float* out_max);

// Inspect/debug helpers (read-only access)
bool ecs_game_get_storage(ecs_entity_t e, int out_counts[RESOURCE_TYPE_COUNT], int* out_capacity);

// Needs called outside of ecs to avoid dependency (currently called in main after ecs_init)
void ecs_register_game_systems(void);
