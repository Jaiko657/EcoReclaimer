#include "opengl_ctx.h"
#include <GLFW/glfw3.h>

static GLFWwindow *s_window = NULL;

void opengl_ctx_set_window(GLFWwindow *window)
{
    s_window = window;
}

GLFWwindow *opengl_ctx_get_window(void)
{
    return s_window;
}
