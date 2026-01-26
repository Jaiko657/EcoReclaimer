#include "engine/input/input.h"

void input_init(void) { }
void input_bind(action_t btn, int keycode) { (void)btn; (void)keycode; }

void input_begin_frame(void) { }

input_t input_for_tick(void)
{
    return (input_t){ .down = 0 };
}

const input_t* input_frame_snapshot(void)
{
    static input_t empty = {0};
    return &empty;
}
