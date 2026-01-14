#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "engine/ecs/ecs.h"

// ===== Global ECS storage (core) =====
extern ComponentMask ecs_mask[ECS_MAX_ENTITIES];
extern uint32_t ecs_gen[ECS_MAX_ENTITIES];
extern uint32_t ecs_next_gen[ECS_MAX_ENTITIES];

// ===== Internal helpers =====
int ent_index_checked(ecs_entity_t e);
int ent_index_unchecked(ecs_entity_t e);
bool ecs_alive_idx(int i);
bool ecs_alive_handle(ecs_entity_t e);
ecs_entity_t handle_from_index(int i);
float clampf(float v, float a, float b);

// ===== Component lifecycle hooks (registered by engine/gameplay modules) =====
typedef void (*ecs_component_hook_fn)(int idx);
extern ecs_component_hook_fn phys_body_create_hook;

void ecs_register_component_destroy_hook(ComponentEnum comp, ecs_component_hook_fn fn);
void ecs_register_phys_body_create_hook(ecs_component_hook_fn fn);
