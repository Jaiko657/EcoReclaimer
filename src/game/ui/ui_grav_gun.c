#include "game/ui/ui_internal.h"
#include "engine/gfx/gfx.h"
#include "engine/renderer/renderer.h"
#include "engine/asset/asset.h"
#include "engine/asset/asset_renderer_internal.h"
#include "game/ecs/helpers/ecs_ui_helpers.h"

#include <stddef.h>
#include <math.h>

#define GRAV_GUN_UI_SCALE 4.0f

static bool grav_gun_ui_get_texture(const gfx_texture** out_tex)
{
    static bool loaded = false;
    static tex_handle_t handle = {0};
    if (!loaded) {
        handle = asset_acquire_texture("assets/images/ui x1.png");
        loaded = true;
    }
    if (!asset_texture_valid(handle)) return false;
    if (out_tex) *out_tex = asset_lookup_texture(handle);
    return true;
}

static bool grav_gun_ui_frames(const gfx_rect** out_frames, int* out_count)
{
    static const gfx_rect frames[] = {
        (gfx_rect){ .x = 65.0f, .y = 77.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 129.0f, .y = 77.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 193.0f, .y = 77.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 257.0f, .y = 77.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 321.0f, .y = 77.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 385.0f, .y = 77.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 449.0f, .y = 77.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 513.0f, .y = 77.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 577.0f, .y = 77.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 0.0f, .y = 109.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 64.0f, .y = 109.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 128.0f, .y = 109.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 192.0f, .y = 109.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 256.0f, .y = 109.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 320.0f, .y = 109.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 384.0f, .y = 109.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 448.0f, .y = 109.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 512.0f, .y = 109.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 576.0f, .y = 109.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 0.0f, .y = 141.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 64.0f, .y = 141.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 128.0f, .y = 141.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 192.0f, .y = 141.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 256.0f, .y = 141.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 320.0f, .y = 141.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 384.0f, .y = 141.0f, .w = 64.0f, .h = 7.0f  },
        (gfx_rect){ .x = 448.0f, .y = 141.0f, .w = 64.0f, .h = 7.0f  }
    };
    if (out_frames) *out_frames = NULL;
    if (out_count) *out_count = 0;
    if (!out_frames || !out_count) return false;
    *out_frames = frames;
    *out_count = (int)(sizeof(frames) / sizeof(frames[0]));
    return true;
}

bool game_ui_grav_gun_metrics(float* out_w, float* out_h)
{
    int frame_count = 0;
    const gfx_rect* frames = NULL;
    if (!grav_gun_ui_frames(&frames, &frame_count) || frame_count < 1) return false;
    if (out_w) *out_w = frames[0].w * GRAV_GUN_UI_SCALE;
    if (out_h) *out_h = frames[0].h * GRAV_GUN_UI_SCALE;
    return true;
}

void game_ui_layer_grav_gun(const render_view_t* view, void* data)
{
    (void)view;
    (void)data;
    int sw = gfx_screen_width();
    int sh = gfx_screen_height();
    float grav_w = 0.0f;
    float grav_h = 0.0f;
    game_ui_grav_gun_metrics(&grav_w, &grav_h);
    float grav_x = (sw - grav_w) * 0.5f;
    float grav_y = sh - grav_h - 10.0f;

    float charge = 0.0f;
    float max_charge = 0.0f;
    if (ecs_grav_gun_get_charge(&charge, &max_charge) && max_charge > 0.0f) {
        const gfx_texture* tex = NULL;
        if (grav_gun_ui_get_texture(&tex)) {
            int frame_count = 0;
            const gfx_rect* frames = NULL;
            grav_gun_ui_frames(&frames, &frame_count);
            if (frames && frame_count > 0) {
                float ratio = charge / max_charge;
                if (ratio < 0.0f) ratio = 0.0f;
                if (ratio > 1.0f) ratio = 1.0f;

                float t = ratio * (float)(frame_count - 1);
                int idx = (int)floorf(t + 0.5f);
                if (idx < 0) idx = 0;
                if (idx >= frame_count) idx = frame_count - 1;

                gfx_rect src = { frames[idx].x, frames[idx].y, frames[idx].w, frames[idx].h };
                gfx_rect dst = { grav_x, grav_y, grav_w, grav_h };
                gfx_vec2 origin = { 0.0f, 0.0f };
                const char* label = "Grav Gun Energy";
                int label_fs = 20;
                int label_w = gfx_measure_text(label, label_fs);
                int label_x = (sw - label_w) / 2;
                int label_y = (int)(grav_y - 28.0f);
                gfx_draw_text(label, label_x, label_y, label_fs, GFX_RAYWHITE);
                gfx_draw_texture_pro(tex, src, dst, origin, 0.0f, GFX_WHITE);
            }
        }
    }
}
