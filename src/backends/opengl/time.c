#include "engine/core/time/time.h"

#include <GLFW/glfw3.h>

static double s_last_time = 0.0;
static float s_frame_dt = 0.0f;

double time_now(void)
{
    return glfwGetTime();
}

float time_frame_dt(void)
{
    return s_frame_dt;
}

int time_fps(void)
{
    if (s_frame_dt <= 0.0f) return 0;
    return (int)(1.0 / (double)s_frame_dt);
}

void time_backend_tick(void)
{
    double now = glfwGetTime();
    s_frame_dt = (float)(now - s_last_time);
    s_last_time = now;
}
