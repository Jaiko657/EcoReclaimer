#include "engine/input/input_backend.h"

void input_backend_bind_defaults(void)
{
}

bool input_backend_is_down(int code)
{
    (void)code;
    return false;
}

bool input_backend_is_pressed(int code)
{
    (void)code;
    return false;
}

input_vec2 input_backend_mouse_pos(void)
{
    return (input_vec2){ .x = 0.0f, .y = 0.0f };
}

float input_backend_mouse_wheel(void)
{
    return 0.0f;
}
