#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "engine/ecs/ecs.h"
#include "shared/utils/build_config.h"

typedef bool (*debug_str_fn)(ecs_entity_t e, char* out, size_t cap);

#if DEBUG_BUILD
bool debug_str_register(ComponentEnum id, debug_str_fn fn);
bool debug_str_component(ComponentEnum id, ecs_entity_t e, char* out, size_t cap);
#else
static inline bool debug_str_register(ComponentEnum id, debug_str_fn fn)
{
    (void)id;
    (void)fn;
    return false;
}
static inline bool debug_str_component(ComponentEnum id, ecs_entity_t e, char* out, size_t cap)
{
    (void)id;
    (void)e;
    (void)out;
    (void)cap;
    return false;
}
#endif
