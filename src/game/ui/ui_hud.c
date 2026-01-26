#include "game/ui/ui_internal.h"
#include "engine/gfx/gfx.h"
#include "engine/renderer/renderer.h"
#include "game/ecs/helpers/ecs_ui_helpers.h"

#include <limits.h>
#include <stdio.h>

static bool get_storage_counts(ecs_entity_t storage, int counts[RESOURCE_TYPE_COUNT], int* out_capacity)
{
    int capacity = 0;
    if (!ecs_storage_get(storage, counts, &capacity)) return false;
    if (out_capacity) *out_capacity = capacity;
    return true;
}

void game_ui_layer_hud(const render_view_t* view, void* data)
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
    int hud_label_y = (int)(grav_y - 28.0f);
    int hud_value_y = hud_label_y + 22;

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
        int label_w = gfx_measure_text(label, 18);
        int value_w = gfx_measure_text(storage_hud, 18);
        int block_w = (label_w > value_w) ? label_w : value_w;
        int x = (int)(grav_x - 20.0f - block_w);
        if (x < 10) x = 10;
        gfx_draw_text(label, x, hud_label_y, 18, GFX_RAYWHITE);
        gfx_draw_text(storage_hud, x, hud_value_y, 18, GFX_RAYWHITE);
    }

    if (get_storage_counts(ecs_storage_find_player(), counts, NULL)) {
        char storage_hud[64];
        snprintf(storage_hud, sizeof(storage_hud), "Recycled: Plastic: %d   |   Metal: %d",
                 counts[RESOURCE_TYPE_PLASTIC], counts[RESOURCE_TYPE_METAL]);
        int tw = gfx_measure_text(storage_hud, 18);
        int x = (int)(grav_x + grav_w + 20.0f);
        if (x + tw > sw - 10) x = sw - tw - 10;
        gfx_draw_text(storage_hud, x, hud_value_y, 18, GFX_RAYWHITE);
    }

    gfx_draw_text("Move: Arrows/WASD | Interact: E | Lift/Throw: C", 10, 10, 18, GFX_GRAY);
}
