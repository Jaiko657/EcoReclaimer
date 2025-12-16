#include "modules/renderer/renderer_internal.h"
#include "modules/ecs/ecs_game.h"
#include "modules/asset/asset.h"
#include "modules/asset/asset_renderer_internal.h"

#include <math.h>
#include <stdio.h>

static bool grav_gun_ui_get_texture(Texture2D* out_tex)
{
    static bool loaded = false;
    static tex_handle_t handle = {0};
    if (!loaded) {
        handle = asset_acquire_texture("assets/images/ui x1.png");
        loaded = true;
    }
    if (!asset_texture_valid(handle)) return false;
    if (out_tex) *out_tex = asset_backend_resolve_texture_value(handle);
    return true;
}

static const rectf* grav_gun_ui_frames(int* out_count)
{
    static const rectf frames[] = {
        (rectf){ 65.0f, 77.0f, 64.0f, 7.0f },
        (rectf){ 129.0f, 77.0f, 64.0f, 7.0f },
        (rectf){ 193.0f, 77.0f, 64.0f, 7.0f },
        (rectf){ 257.0f, 77.0f, 64.0f, 7.0f },
        (rectf){ 321.0f, 77.0f, 64.0f, 7.0f },
        (rectf){ 385.0f, 77.0f, 64.0f, 7.0f },
        (rectf){ 449.0f, 77.0f, 64.0f, 7.0f },
        (rectf){ 513.0f, 77.0f, 64.0f, 7.0f },
        (rectf){ 577.0f, 77.0f, 64.0f, 7.0f },
        (rectf){ 0.0f, 109.0f, 64.0f, 7.0f },
        (rectf){ 64.0f, 109.0f, 64.0f, 7.0f },
        (rectf){ 128.0f, 109.0f, 64.0f, 7.0f },
        (rectf){ 192.0f, 109.0f, 64.0f, 7.0f },
        (rectf){ 256.0f, 109.0f, 64.0f, 7.0f },
        (rectf){ 320.0f, 109.0f, 64.0f, 7.0f },
        (rectf){ 384.0f, 109.0f, 64.0f, 7.0f },
        (rectf){ 448.0f, 109.0f, 64.0f, 7.0f },
        (rectf){ 512.0f, 109.0f, 64.0f, 7.0f },
        (rectf){ 576.0f, 109.0f, 64.0f, 7.0f },
        (rectf){ 0.0f, 141.0f, 64.0f, 7.0f },
        (rectf){ 64.0f, 141.0f, 64.0f, 7.0f },
        (rectf){ 128.0f, 141.0f, 64.0f, 7.0f },
        (rectf){ 192.0f, 141.0f, 64.0f, 7.0f },
        (rectf){ 256.0f, 141.0f, 64.0f, 7.0f },
        (rectf){ 320.0f, 141.0f, 64.0f, 7.0f },
        (rectf){ 384.0f, 141.0f, 64.0f, 7.0f },
        (rectf){ 448.0f, 141.0f, 64.0f, 7.0f }
    };
    if (out_count) *out_count = (int)(sizeof(frames) / sizeof(frames[0]));
    return frames;
}

void draw_screen_space_ui(const render_view_t* view)
{
    renderer_debug_draw_ui(view);

    // ===== toast =====
    if (ecs_toast_is_active()) {
        const char* t = ecs_toast_get_text();
        const int fs = 20;
        int tw = MeasureText(t, fs);
        int x = (GetScreenWidth() - tw)/2;
        int y = 10;

        DrawRectangle(x-8, y-4, tw+16, 28, (Color){0,0,0,180});
        DrawText(t, x, y, fs, RAYWHITE);
    }

    // ===== HUD =====
    {
        int counts[RESOURCE_TYPE_COUNT] = {0};
        int capacity = 0;
        if (game_get_tardas_storage(counts, &capacity)) {
            int total = counts[RESOURCE_TYPE_PLASTIC] + counts[RESOURCE_TYPE_METAL];
            char storage_hud[64];
            snprintf(storage_hud, sizeof(storage_hud), "TARDAS P:%d M:%d (%d/%d)",
                     counts[RESOURCE_TYPE_PLASTIC], counts[RESOURCE_TYPE_METAL], total, capacity);
            DrawText(storage_hud, 10, 10, 18, RAYWHITE);
            DrawText("Move: Arrows/WASD | Interact: E | Lift/Throw: C", 10, 32, 18, GRAY);
        } else {
            DrawText("Move: Arrows/WASD | Interact: E | Lift/Throw: C", 10, 10, 18, GRAY);
        }
    }

    // ===== grav gun charge UI =====
    {
        float charge = 0.0f;
        float max_charge = 0.0f;
        if (game_get_grav_gun_charge(&charge, &max_charge) && max_charge > 0.0f) {
            Texture2D tex;
            if (grav_gun_ui_get_texture(&tex)) {
                int frame_count = 0;
                const rectf* frames = grav_gun_ui_frames(&frame_count);
                if (frames && frame_count > 0) {
                    float ratio = charge / max_charge;
                    if (ratio < 0.0f) ratio = 0.0f;
                    if (ratio > 1.0f) ratio = 1.0f;

                    float t = ratio * (float)(frame_count - 1);
                    int idx = (int)floorf(t + 0.5f);
                    if (idx < 0) idx = 0;
                    if (idx >= frame_count) idx = frame_count - 1;

                    int sw = GetScreenWidth();
                    Rectangle src = { frames[idx].x, frames[idx].y, frames[idx].w, frames[idx].h };
                    Rectangle dst = { (float)(sw - 138), 10.0f, frames[idx].w * 2.0f, frames[idx].h * 2.0f };
                    Vector2 origin = { 0.0f, 0.0f };
                    DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
                }
            }
        }
    }

    // ===== floating billboards (from proximity) =====
    {
        const int fs = 15; // slightly larger text
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        Rectangle screen_bounds = {0, 0, (float)sw, (float)sh};

        for (ecs_billboard_iter_t it = ecs_billboards_begin(); ; ) {
            ecs_billboard_view_t v;
            if (!ecs_billboards_next(&it, &v)) break;

            Vector2 world_pos = { v.x, v.y + v.y_offset };
            Vector2 screen_pos = GetWorldToScreen2D(world_pos, view->cam);

            int tw = MeasureText(v.text, fs);
            float bb_w = (float)(tw + 12);
            float bb_h = 26.0f;
            Rectangle bb = {
                screen_pos.x - (bb_w - 12.0f) / 2.0f,
                screen_pos.y - 6.0f,
                bb_w,
                bb_h
            };

            if (!rects_intersect(bb, screen_bounds)) continue;

            int x = (int)(bb.x + 6.0f);
            int y = (int)screen_pos.y;

            unsigned char a = u8(v.alpha);
            Color bg = (Color){ 0, 0, 0, (unsigned char)(a * 120 / 255) }; // softer background
            Color fg = (Color){ 255, 255, 255, a };

            DrawRectangle((int)bb.x, (int)bb.y, (int)bb.width, (int)bb.height, bg);
            DrawText(v.text, x, y, fs, fg);
        }
    }
}
