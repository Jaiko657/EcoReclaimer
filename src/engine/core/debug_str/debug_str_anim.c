#include "engine/core/debug_str/debug_str_registry.h"

#if DEBUG_BUILD

#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_engine.h"
#include <stdio.h>

static bool debug_str_anim(ecs_entity_t e, char* out, size_t cap)
{
    int idx = ent_index_checked(e);
    if (idx < 0 || !out || cap == 0) return false;
    const cmp_anim_t* a = &cmp_anim[idx];
    return snprintf(out, cap, "ANIM(frame=%dx%d, anim=%d/%d, frame_i=%d, t=%.3f)",
                    a->frame_w, a->frame_h, a->current_anim, a->anim_count, a->frame_index,
                    a->current_time) > 0;
}

void debug_str_register_anim(void)
{
    debug_str_register(ENUM_ANIM, debug_str_anim);
}

#endif
