#include "engine/renderer/renderer_internal.h"
#include "engine/runtime/camera.h"

static gfx_rect camera_view_rect(const gfx_camera2d* cam)
{
    float viewW = (float)gfx_screen_width()  / cam->zoom;
    float viewH = (float)gfx_screen_height() / cam->zoom;
    float left  = cam->target.x - cam->offset.x / cam->zoom;
    float top   = cam->target.y - cam->offset.y / cam->zoom;
    return (gfx_rect){ .x = left, .y = top, .w = viewW, .h = viewH  };
}

render_view_t build_camera_view(void)
{
    camera_view_t logical = camera_get_view();
    int sw = gfx_screen_width();
    int sh = gfx_screen_height();

    gfx_camera2d cam = {
        .target   = (gfx_vec2){ .x = logical.center.x, .y = logical.center.y  },
        .offset   = (gfx_vec2){ .x = sw / 2.0f, .y = sh / 2.0f  },
        .rotation = 0.0f,
        .zoom     = logical.zoom > 0.0f ? logical.zoom : 1.0f
    };

    gfx_rect view = camera_view_rect(&cam);
    float margin = logical.padding >= 0.0f ? logical.padding : 0.0f;
    gfx_rect padded = expand_rect(view, margin);

    return (render_view_t){ .cam = cam, .view = view, .padded_view = padded };
}
