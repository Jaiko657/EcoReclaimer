#include "game/ecs/ecs_game.h"

#include "engine/core/logger.h"
#include "engine/asset/asset.h"
#include "engine/asset/asset_renderer_internal.h"
#include "engine/renderer/renderer_internal.h"
#include "game/ecs/ecs_gravity_gun.h"
#include "game/ecs/ecs_storage.h"
#include <limits.h>
#include <math.h>
#include <stddef.h>
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

static bool grav_gun_ui_frames(const rectf** out_frames, int* out_count)
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
    if (out_frames) *out_frames = NULL;
    if (out_count) *out_count = 0;
    if (!out_frames || !out_count) return false;
    *out_frames = frames;
    *out_count = (int)(sizeof(frames) / sizeof(frames[0]));
    return true;
}

static bool get_storage_counts(ecs_entity_t storage, int counts[RESOURCE_TYPE_COUNT], int* out_capacity)
{
    int capacity = 0;
    if (!ecs_storage_get(storage, counts, &capacity)) return false;
    if (out_capacity) *out_capacity = capacity;
    return true;
}

void ecs_game_render_ui(const render_view_t* view)
{
    (void)view;
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    float grav_scale = 4.0f;
    int grav_frame_count = 0;
    const rectf* grav_frames = NULL;
    grav_gun_ui_frames(&grav_frames, &grav_frame_count);
    float grav_w = (grav_frames && grav_frame_count > 0) ? grav_frames[0].w * grav_scale : 0.0f;
    float grav_h = (grav_frames && grav_frame_count > 0) ? grav_frames[0].h * grav_scale : 0.0f;
    float grav_x = (sw - grav_w) * 0.5f;
    float grav_y = sh - grav_h - 10.0f;
    int hud_label_y = (int)(grav_y - 28.0f);
    int hud_value_y = hud_label_y + 22;

    // ===== HUD =====
    {
        int counts[RESOURCE_TYPE_COUNT] = {0};
        int capacity = 0;

        if (get_storage_counts(ecs_storage_find_tardas(), counts, &capacity)) {
            const char* label = "Items Stored in TARDAS";
            int total = counts[RESOURCE_TYPE_PLASTIC] + counts[RESOURCE_TYPE_METAL];
            char storage_hud[64];
            if (capacity >= INT_MAX / 2) {
                snprintf(storage_hud, sizeof(storage_hud), "Plastic: %d | Metal: %d (%d/inf)",
                         counts[RESOURCE_TYPE_PLASTIC], counts[RESOURCE_TYPE_METAL], total);
            } else {
                snprintf(storage_hud, sizeof(storage_hud), "Plastic: %d | Metal: %d (%d/%d)",
                         counts[RESOURCE_TYPE_PLASTIC], counts[RESOURCE_TYPE_METAL], total, capacity);
            }
            int label_w = MeasureText(label, 18);
            int value_w = MeasureText(storage_hud, 18);
            int block_w = (label_w > value_w) ? label_w : value_w;
            int x = (int)(grav_x - 20.0f - block_w);
            if (x < 10) x = 10;
            DrawText(label, x, hud_label_y, 18, RAYWHITE);
            DrawText(storage_hud, x, hud_value_y, 18, RAYWHITE);
        }

        if (get_storage_counts(ecs_storage_find_player(), counts, NULL)) {
            char storage_hud[64];
            snprintf(storage_hud, sizeof(storage_hud), "Recycled: Plastic: %d   |   Metal: %d",
                     counts[RESOURCE_TYPE_PLASTIC], counts[RESOURCE_TYPE_METAL]);
            int tw = MeasureText(storage_hud, 18);
            int x = (int)(grav_x + grav_w + 20.0f);
            if (x + tw > sw - 10) x = sw - tw - 10;
            DrawText(storage_hud, x, hud_value_y, 18, RAYWHITE);
        }

        DrawText("Move: Arrows/WASD | Interact: E | Lift/Throw: C", 10, 10, 18, GRAY);
    }

    // ===== grav gun charge UI =====
    {
        float charge = 0.0f;
        float max_charge = 0.0f;
        if (ecs_grav_gun_get_charge(&charge, &max_charge) && max_charge > 0.0f) {
            Texture2D tex;
            if (grav_gun_ui_get_texture(&tex)) {
                int frame_count = 0;
                const rectf* frames = NULL;
                grav_gun_ui_frames(&frames, &frame_count);
                if (frames && frame_count > 0) {
                    float ratio = charge / max_charge;
                    if (ratio < 0.0f) ratio = 0.0f;
                    if (ratio > 1.0f) ratio = 1.0f;

                    float t = ratio * (float)(frame_count - 1);
                    int idx = (int)floorf(t + 0.5f);
                    if (idx < 0) idx = 0;
                    if (idx >= frame_count) idx = frame_count - 1;

                    Rectangle src = { frames[idx].x, frames[idx].y, frames[idx].w, frames[idx].h };
                    Rectangle dst = { grav_x, grav_y, grav_w, grav_h };
                    Vector2 origin = { 0.0f, 0.0f };
                    const char* label = "Grav Gun Energy";
                    int label_fs = 20;
                    int label_w = MeasureText(label, label_fs);
                    int label_x = (sw - label_w) / 2;
                    int label_y = (int)(grav_y - 28.0f);
                    DrawText(label, label_x, label_y, label_fs, RAYWHITE);
                    DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
                }
            }
        }
    }
}
