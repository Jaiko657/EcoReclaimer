#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "engine/gfx/gfx.h"

enum { FX_MAX_LINES = 128 };

typedef struct {
    gfx_vec2 a;
    gfx_vec2 b;
    float width;
    gfx_color color;
} fx_line_t;

void fx_lines_clear(void);
bool fx_line_push(gfx_vec2 a, gfx_vec2 b, float width, gfx_color color);
size_t fx_line_count(void);
const fx_line_t* fx_line_at(size_t idx);
