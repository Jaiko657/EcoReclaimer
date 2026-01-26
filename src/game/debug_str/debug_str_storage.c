#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "game/ecs/helpers/ecs_storage_helpers.h"
#include <stdio.h>

static bool debug_str_storage(ecs_entity_t e, char* out, size_t cap)
{
    if (!out || cap == 0) return false;
    int counts[RESOURCE_TYPE_COUNT] = {0};
    int capacity = 0;
    if (!ecs_storage_get(e, counts, &capacity)) {
        return snprintf(out, cap, "STORAGE(capacity=?)") > 0;
    }
    return snprintf(out, cap, "STORAGE(plastic=%d, metal=%d, capacity=%d)",
                    counts[RESOURCE_TYPE_PLASTIC], counts[RESOURCE_TYPE_METAL], capacity) > 0;
}

void debug_str_register_storage(void)
{
    debug_str_register(ENUM_STORAGE, debug_str_storage);
}

#endif
