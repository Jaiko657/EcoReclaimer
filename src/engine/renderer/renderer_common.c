#include "engine/renderer/renderer_internal.h"

#include <math.h>

unsigned char u8(float x)
{
    if (x < 0.f) x = 0.f;
    if (x > 1.f) x = 1.f;
    return (unsigned char)(x * 255.0f + 0.5f);
}

gfx_rect expand_rect(gfx_rect r, float margin)
{
    return (gfx_rect){
        r.x - margin,
        r.y - margin,
        r.w + 2.0f * margin,
        r.h + 2.0f * margin
    };
}

gfx_rect intersect_rect(gfx_rect a, gfx_rect b)
{
    float nx = fmaxf(a.x, b.x);
    float ny = fmaxf(a.y, b.y);
    float nw = fminf(a.x + a.w,  b.x + b.w)  - nx;
    float nh = fminf(a.y + a.h, b.y + b.h) - ny;
    if (nw <= 0.0f || nh <= 0.0f) {
        return (gfx_rect){ .x = 0.0f, .y = 0.0f, .w = 0.0f, .h = 0.0f };
    }
    return (gfx_rect){ .x = nx, .y = ny, .w = nw, .h = nh  };
}

bool rects_intersect(gfx_rect a, gfx_rect b)
{
    return a.x < b.x + b.w && a.x + a.w > b.x &&
           a.y < b.y + b.h && a.y + a.h > b.y;
}

gfx_rect sprite_bounds(const ecs_sprite_view_t* v)
{
    float w = fabsf(v->src.w);
    float h = fabsf(v->src.h);
    return (gfx_rect){ .x = v->x - v->ox, .y = v->y - v->oy, .w = w, .h = h  };
}
