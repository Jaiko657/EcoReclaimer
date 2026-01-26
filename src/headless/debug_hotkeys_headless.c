#include "shared/utils/build_config.h"

#if DEBUG_BUILD

#include "engine/input/input.h"
#include "engine/engine/engine_scheduler/engine_scheduler_registration.h"
#include "engine/debug/debug_hotkeys/debug_hotkeys.h"

void sys_debug_binds(const input_t* in)
{
    (void)in;
}

void debug_post_frame(void)
{
}

SYSTEMS_ADAPT_INPUT(sys_debug_binds_adapt, sys_debug_binds)

#endif
