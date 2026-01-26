#include "engine/runtime/effects.h"

static fx_line_t g_lines[FX_MAX_LINES];
static size_t g_line_count = 0;

void fx_lines_clear(void)
{
    g_line_count = 0;
}

bool fx_line_push(gfx_vec2 a, gfx_vec2 b, float width, gfx_color color)
{
    if (g_line_count >= FX_MAX_LINES) return false;
    g_lines[g_line_count++] = (fx_line_t){
        .a = a,
        .b = b,
        .width = width,
        .color = color,
    };
    return true;
}

size_t fx_line_count(void)
{
    return g_line_count;
}

const fx_line_t* fx_line_at(size_t idx)
{
    if (idx >= g_line_count) return NULL;
    return &g_lines[idx];
}
