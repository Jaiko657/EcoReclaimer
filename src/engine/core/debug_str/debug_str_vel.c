#include "engine/core/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_engine.h"
#include <stdio.h>

static bool debug_str_vel(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_velocity_t* v = &cmp_vel[idx];
    return snprintf(out, cap, "VEL(x=%.3f, y=%.3f, raw=%d, face=%d)",
                    v->x, v->y, v->facing.rawDir, v->facing.facingDir) > 0;
}

void debug_str_register_vel(void)
{
    debug_str_register(ENUM_VEL, debug_str_vel);
}

#endif
