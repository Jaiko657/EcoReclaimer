#include "engine/debug/debug_str/debug_str_engine.h"

#if DEBUG_BUILD

void debug_str_register_pos(void);
void debug_str_register_vel(void);
void debug_str_register_phys_body(void);
void debug_str_register_spr(void);
void debug_str_register_anim(void);
void debug_str_register_col(void);
void debug_str_register_trigger(void);
void debug_str_register_billboard(void);

void debug_str_engine_register_all(void)
{
    debug_str_register_pos();
    debug_str_register_vel();
    debug_str_register_phys_body();
    debug_str_register_spr();
    debug_str_register_anim();
    debug_str_register_col();
    debug_str_register_trigger();
    debug_str_register_billboard();
}

#endif
