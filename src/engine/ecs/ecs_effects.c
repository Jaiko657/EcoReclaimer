#include "engine/ecs/ecs_engine.h"
#include "engine/runtime/effects.h"
#include "engine/input/input.h"
#include "engine/engine/engine_scheduler/engine_scheduler_registration.h"

static void sys_effects_tick_begin_impl(void)
{
    fx_lines_clear();
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_SPR) == 0) continue;
        cmp_spr[i].fx.highlighted = false;
        cmp_spr[i].fx.front = false;
    }
}

SYSTEMS_ADAPT_VOID(sys_effects_tick_begin_adapt, sys_effects_tick_begin_impl)