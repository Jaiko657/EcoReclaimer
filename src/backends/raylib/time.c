#include "engine/core/time/time.h"

#include "raylib.h"

double time_now(void)
{
    return GetTime();
}

float time_frame_dt(void)
{
    return GetFrameTime();
}

int time_fps(void)
{
    return GetFPS();
}
