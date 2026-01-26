#pragma once

double GetTime(void);
float GetFrameTime(void);
int GetFPS(void);

void raylib_time_stub_reset(void);
void raylib_time_stub_set_time(double t);
void raylib_time_stub_set_frame(float dt);
void raylib_time_stub_set_fps(int fps);
