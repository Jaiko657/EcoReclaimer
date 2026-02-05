#include "engine/input/input_backend.h"

#if !defined(HEADLESS)

#include "raylib.h"

static bool is_mouse_code(int code)
{
    return code >= MOUSE_BUTTON_LEFT && code <= MOUSE_BUTTON_BACK;
}

void input_backend_bind_defaults(void)
{
    /* Movement: Arrows + WASD */
    input_bind(ACT_LEFT,  KEY_LEFT);   input_bind(ACT_LEFT,  KEY_A);
    input_bind(ACT_RIGHT, KEY_RIGHT);  input_bind(ACT_RIGHT, KEY_D);
    input_bind(ACT_UP,    KEY_UP);     input_bind(ACT_UP,    KEY_W);
    input_bind(ACT_DOWN,  KEY_DOWN);   input_bind(ACT_DOWN,  KEY_S);

    /* Interact: E */
    input_bind(ACT_INTERACT, KEY_E);

    /* Lift/Throw: C */
    input_bind(ACT_LIFT, KEY_C);

    /* Mouse buttons */
    input_bind(ACT_MOUSE_L, MOUSE_LEFT_BUTTON);
    input_bind(ACT_MOUSE_R, MOUSE_RIGHT_BUTTON);

#if DEBUG_BUILD
    input_bind(ACT_DEBUG_ASSET_PRINT, KEY_SPACE);

    /* Debug toggles */
    input_bind(ACT_DEBUG_COLLIDER_ECS,     KEY_ONE);
    input_bind(ACT_DEBUG_COLLIDER_PHYSICS, KEY_TWO);
    input_bind(ACT_DEBUG_COLLIDER_STATIC,  KEY_THREE);
    input_bind(ACT_DEBUG_TRIGGERS,         KEY_FOUR);
    input_bind(ACT_DEBUG_INSPECT,          KEY_FIVE);
    input_bind(ACT_DEBUG_RELOAD_TMX,       KEY_R);
    input_bind(ACT_DEBUG_FPS,              KEY_GRAVE);
    input_bind(ACT_DEBUG_TRACE_START,      KEY_F9);
    input_bind(ACT_DEBUG_TRACE_STOP,       KEY_F10);
    input_bind(ACT_DEBUG_SCREENSHOT,       KEY_PRINT_SCREEN);
#endif
}

bool input_backend_is_down(int code)
{
    if (is_mouse_code(code)) return IsMouseButtonDown(code);
    if (code >= KEY_NULL && code <= KEY_KB_MENU) return IsKeyDown(code);
    return false;
}

bool input_backend_is_pressed(int code)
{
    if (is_mouse_code(code)) return IsMouseButtonPressed(code);
    if (code >= KEY_NULL && code <= KEY_KB_MENU) return IsKeyPressed(code);
    return false;
}

input_vec2 input_backend_mouse_pos(void)
{
    Vector2 m = GetMousePosition();
    return (input_vec2){ .x = m.x, .y = m.y };
}

float input_backend_mouse_wheel(void)
{
    return GetMouseWheelMove();
}

#endif
