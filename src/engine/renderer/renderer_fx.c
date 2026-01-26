#include "engine/renderer/renderer_internal.h"
#include "engine/runtime/effects.h"

#include <math.h>

void draw_effect_lines(const render_view_t* view)
{
    if (!view) return;

    size_t count = fx_line_count();
    if (count == 0) return;

    for (size_t i = 0; i < count; ++i) {
        const fx_line_t* line = fx_line_at(i);
        if (!line) continue;

        float minx = fminf(line->a.x, line->b.x);
        float miny = fminf(line->a.y, line->b.y);
        float maxx = fmaxf(line->a.x, line->b.x);
        float maxy = fmaxf(line->a.y, line->b.y);
        gfx_rect bounds = { minx, miny, maxx - minx, maxy - miny };
        if (!rects_intersect(bounds, view->padded_view)) continue;

        float width = (line->width > 0.0f) ? line->width : 1.0f;
        gfx_draw_line(
            (gfx_vec2){ .x = line->a.x, .y = line->a.y  },
            (gfx_vec2){ .x = line->b.x, .y = line->b.y  },
            width,
            line->color
        );
    }
}
