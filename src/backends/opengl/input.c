#include "engine/input/input_backend.h"
#include "engine/input/input.h"
#include "opengl_ctx.h"

#include <GLFW/glfw3.h>
#include <stdbool.h>

static bool was_down[512];
static bool key_states[512];
static float s_mouse_wheel = 0.0f;

void input_backend_scroll_callback(double xoffset, double yoffset)
{
    (void)xoffset;
    s_mouse_wheel += (float)yoffset;
}

void input_backend_bind_defaults(void)
{
    input_bind(ACT_LEFT,  GLFW_KEY_LEFT);
    input_bind(ACT_LEFT,  GLFW_KEY_A);
    input_bind(ACT_RIGHT, GLFW_KEY_RIGHT);
    input_bind(ACT_RIGHT, GLFW_KEY_D);
    input_bind(ACT_UP,    GLFW_KEY_UP);
    input_bind(ACT_UP,    GLFW_KEY_W);
    input_bind(ACT_DOWN,  GLFW_KEY_DOWN);
    input_bind(ACT_DOWN,  GLFW_KEY_S);

    input_bind(ACT_INTERACT, GLFW_KEY_E);
    input_bind(ACT_LIFT,     GLFW_KEY_C);

    input_bind(ACT_MOUSE_L, GLFW_MOUSE_BUTTON_LEFT);
    input_bind(ACT_MOUSE_R, GLFW_MOUSE_BUTTON_RIGHT);

#if DEBUG_BUILD
    input_bind(ACT_DEBUG_ASSET_PRINT, GLFW_KEY_SPACE);
    input_bind(ACT_DEBUG_COLLIDER_ECS,     GLFW_KEY_1);
    input_bind(ACT_DEBUG_COLLIDER_PHYSICS, GLFW_KEY_2);
    input_bind(ACT_DEBUG_COLLIDER_STATIC,  GLFW_KEY_3);
    input_bind(ACT_DEBUG_TRIGGERS,         GLFW_KEY_4);
    input_bind(ACT_DEBUG_INSPECT,          GLFW_KEY_5);
    input_bind(ACT_DEBUG_RELOAD_TMX,       GLFW_KEY_R);
    input_bind(ACT_DEBUG_FPS,              GLFW_KEY_GRAVE_ACCENT);
    input_bind(ACT_DEBUG_TRACE_START,      GLFW_KEY_F9);
    input_bind(ACT_DEBUG_TRACE_STOP,       GLFW_KEY_F10);
    input_bind(ACT_DEBUG_SCREENSHOT,       GLFW_KEY_PRINT_SCREEN);
#endif
}

void input_backend_frame_sync(void)
{
    GLFWwindow *win = opengl_ctx_get_window();
    if (!win) return;
    for (int i = 0; i < 512; i++) {
        bool down = false;
        if (i >= GLFW_MOUSE_BUTTON_1 && i <= GLFW_MOUSE_BUTTON_LAST) {
            down = glfwGetMouseButton(win, i) == GLFW_PRESS;
        } else {
            down = glfwGetKey(win, i) == GLFW_PRESS;
        }
        was_down[i] = key_states[i];
        key_states[i] = down;
    }
}

bool input_backend_is_down(int code)
{
    if (code < 0 || code >= 512) return false;
    return key_states[code];
}

bool input_backend_is_pressed(int code)
{
    if (code < 0 || code >= 512) return false;
    return key_states[code] && !was_down[code];
}

input_vec2 input_backend_mouse_pos(void)
{
    GLFWwindow *win = opengl_ctx_get_window();
    if (!win) return (input_vec2){ .x = 0.0f, .y = 0.0f };
    double x = 0.0, y = 0.0;
    glfwGetCursorPos(win, &x, &y);
    return (input_vec2){ .x = (float)x, .y = (float)y };
}

float input_backend_mouse_wheel(void)
{
    float v = s_mouse_wheel;
    s_mouse_wheel = 0.0f;
    return v;
}
