#include "engine/debug/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_engine.h"
#include <inttypes.h>
#include <stdio.h>

static bool debug_str_trigger(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_trigger_t* t = &cmp_trigger[idx];
    return snprintf(out, cap, "TRIGGER(pad=%.2f, target=0x%016" PRIX64 ")",
                    t->pad, (uint64_t)t->target_mask) > 0;
}

void debug_str_register_trigger(void)
{
    debug_str_register(ENUM_TRIGGER, debug_str_trigger);
}

#endif
