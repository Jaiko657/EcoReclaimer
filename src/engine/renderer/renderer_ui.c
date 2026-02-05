#include "engine/renderer/renderer_internal.h"
#include "engine/runtime/toast.h"
#include "engine/renderer/renderer.h"
#include "engine/utils/dynarray.h"

typedef struct {
    renderer_ui_layer_fn fn;
    void* data;
    int order;
} renderer_ui_layer_t;

static DA(renderer_ui_layer_t) g_ui_layers = {0};

static void renderer_ui_layer_toast(const render_view_t* view, void* data)
{
    (void)view;
    (void)data;

    if (!ecs_toast_is_active()) return;
    const char* t = ecs_toast_get_text();
    const int fs = 20;
    int sw = gfx_screen_width();
    int tw = gfx_measure_text(t, fs);
    int x = (sw - tw) / 2;
    int y = 10;

    gfx_draw_rect((gfx_rect){ .x = (float)(x - 8), .y = (float)(y - 4), .w = (float)(tw + 16), .h = 28.0f  }, GFX_COLOR(0, 0, 0, 180));
    gfx_draw_text(t, x, y, fs, GFX_RAYWHITE);
}

void renderer_ui_registry_init(void)
{
    DA_CLEAR(&g_ui_layers);
    renderer_ui_register_layer(renderer_ui_layer_toast, NULL, -100);
}

static void sort_ui_layers(void)
{
    size_t n = g_ui_layers.size;
    for (size_t i = 1; i < n; ++i) {
        renderer_ui_layer_t key = g_ui_layers.data[i];
        size_t j = i;
        while (j > 0 && g_ui_layers.data[j - 1].order > key.order) {
            g_ui_layers.data[j] = g_ui_layers.data[j - 1];
            --j;
        }
        g_ui_layers.data[j] = key;
    }
}

void renderer_ui_register_layer(renderer_ui_layer_fn fn, void* data, int order)
{
    if (!fn) return;
    renderer_ui_layer_t entry = { .fn = fn, .data = data, .order = order };
    DA_APPEND(&g_ui_layers, entry);
    sort_ui_layers();
}

static void renderer_ui_run_layers(const render_view_t* view)
{
    for (size_t i = 0; i < g_ui_layers.size; ++i) {
        renderer_ui_layer_t entry = g_ui_layers.data[i];
        entry.fn(view, entry.data);
    }
}

void draw_screen_space_ui(const render_view_t* view)
{
    renderer_debug_draw_ui(view);

    // ===== floating billboards (from proximity) =====
    {
        const int fs = 15; // slightly larger text
        int sw = gfx_screen_width();
        int sh = gfx_screen_height();
        gfx_rect screen_bounds = {0, 0, (float)sw, (float)sh};

        for (ecs_billboard_iter_t it = ecs_billboards_begin(); ; ) {
            ecs_billboard_view_t v;
            if (!ecs_billboards_next(&it, &v)) break;

            gfx_vec2 world_pos = { v.x, v.y + v.y_offset };
            gfx_vec2 screen_pos = gfx_world_to_screen(world_pos, &view->cam);

            int tw = gfx_measure_text(v.text, fs);
            float bb_w = (float)(tw + 12);
            float bb_h = 26.0f;
            gfx_rect bb = {
                screen_pos.x - (bb_w - 12.0f) / 2.0f,
                screen_pos.y - 6.0f,
                bb_w,
                bb_h
            };

            if (!rects_intersect(bb, screen_bounds)) continue;

            int x = (int)(bb.x + 6.0f);
            int y = (int)screen_pos.y;

            unsigned char a = u8(v.alpha);
            float alpha = (float)a / 255.0f;
            gfx_color bg = (gfx_color){ .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = alpha * (120.0f / 255.0f)  }; // softer background
            gfx_color fg = (gfx_color){ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = alpha  };

            gfx_draw_rect(bb, bg);
            gfx_draw_text(v.text, x, y, fs, fg);
        }
    }
    renderer_ui_run_layers(view);
}
