#pragma once
#include <stdint.h>
#include "engine/ecs/ecs_physics_types.h"

// Internal helpers (component indices)
void ecs_phys_body_create_for_entity(int idx);
void ecs_phys_body_destroy_for_entity(int idx);
void ecs_phys_destroy_all(void);

// Returns a stable bit for the given tag name, creating it if needed.
unsigned int phys_tag_bit(const char* name);

// Parses a '|' or comma separated list of tags into bit flags.
// Special tokens: "all" -> 0xFFFFFFFFu, "none" -> 0u.
unsigned int phys_parse_tag_list(const char* s);

// Resets the registry (useful for tests).
void phys_tag_reset_registry(void);
