#pragma once

#include <stdbool.h>
#include "engine/input/input.h"

// Backend-specific defaults (Raylib, SDL, etc.).
void input_backend_bind_defaults(void);

// Physical input queries for backend key/mouse codes.
bool input_backend_is_down(int code);
bool input_backend_is_pressed(int code);
input_vec2 input_backend_mouse_pos(void);
float input_backend_mouse_wheel(void);
