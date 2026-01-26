#include "shared/utils/build_config.h"
#include "engine/renderer/renderer.h"
#include "engine/renderer/renderer_internal.h"
#include "engine/world/world_query.h"
#include "engine/core/time/time.h"

#include <math.h>
#include <stdio.h>

#if DEBUG_BUILD

#if DEBUG_COLLISION
static bool g_draw_ecs_colliders = false;
static bool g_draw_phys_colliders = false;
static bool g_draw_static_colliders = false;
#endif

#if DEBUG_TRIGGERS
static bool g_draw_triggers = false;
#endif

#if DEBUG_FPS
static bool g_show_fps = false;
#endif

#if DEBUG_COLLISION
bool renderer_toggle_ecs_colliders(void)
{
    g_draw_ecs_colliders = !g_draw_ecs_colliders;
    return g_draw_ecs_colliders;
}

bool renderer_toggle_phys_colliders(void)
{
    g_draw_phys_colliders = !g_draw_phys_colliders;
    return g_draw_phys_colliders;
}

bool renderer_toggle_static_colliders(void)
{
    g_draw_static_colliders = !g_draw_static_colliders;
    return g_draw_static_colliders;
}
#else
bool renderer_toggle_ecs_colliders(void) { return false; }
bool renderer_toggle_phys_colliders(void) { return false; }
bool renderer_toggle_static_colliders(void) { return false; }
#endif

#if DEBUG_TRIGGERS
bool renderer_toggle_triggers(void)
{
    g_draw_triggers = !g_draw_triggers;
    return g_draw_triggers;
}
#else
bool renderer_toggle_triggers(void) { return false; }
#endif

#if DEBUG_FPS
bool renderer_toggle_fps_overlay(void)
{
    g_show_fps = !g_show_fps;
    return g_show_fps;
}
#else
bool renderer_toggle_fps_overlay(void) { return false; }
#endif

#endif

#if DEBUG_BUILD && DEBUG_COLLISION
static gfx_rect collider_bounds_at(float x, float y, float hx, float hy)
{
    return (gfx_rect){ .x = x - hx, .y = y - hy, .w = 2.f * hx, .h = 2.f * hy  };
}

static void draw_collider_outline(gfx_rect bounds, const gfx_rect* padded_view, gfx_color color)
{
    if (!rects_intersect(bounds, *padded_view)) return;
    gfx_draw_rect_lines(bounds, color);
}

static void draw_static_colliders(const render_view_t* view, gfx_color color)
{
    int tiles_w = 0, tiles_h = 0;
    if (!world_size_tiles(&tiles_w, &tiles_h)) return;
    int tile_px = world_tile_size();
    int subtile_px = world_subtile_size();
    if (tiles_w <= 0 || tiles_h <= 0 || tile_px <= 0 || subtile_px <= 0) return;

    int subtiles_per_tile = tile_px / subtile_px;
    if (subtiles_per_tile <= 0) return;

    for (int ty = 0; ty < tiles_h; ++ty) {
        for (int tx = 0; tx < tiles_w; ++tx) {
            if (world_tile_is_dynamic(tx, ty)) continue;

            gfx_rect tile_rect = { (float)(tx * tile_px), (float)(ty * tile_px), (float)tile_px, (float)tile_px };
            if (!rects_intersect(tile_rect, view->padded_view)) continue;

            uint16_t mask = world_subtile_mask_at(tx, ty);
            if (mask == 0) continue;

            for (int sy = 0; sy < subtiles_per_tile; ++sy) {
                for (int sx = 0; sx < subtiles_per_tile; ++sx) {
                    int bit = sy * subtiles_per_tile + sx;
                    if ((mask & (uint16_t)(1u << bit)) == 0) continue;

                    gfx_rect r = {
                        tile_rect.x + (float)(sx * subtile_px),
                        tile_rect.y + (float)(sy * subtile_px),
                        (float)subtile_px,
                        (float)subtile_px
                    };
                    draw_collider_outline(r, &view->padded_view, color);
                }
            }
        }
    }
}

static void draw_dynamic_colliders(const render_view_t* view, gfx_color color)
{
    int tiles_w = 0, tiles_h = 0;
    if (!world_size_tiles(&tiles_w, &tiles_h)) return;
    int tile_px = world_tile_size();
    int subtile_px = world_subtile_size();
    if (tiles_w <= 0 || tiles_h <= 0 || tile_px <= 0 || subtile_px <= 0) return;

    int subtiles_per_tile = tile_px / subtile_px;
    if (subtiles_per_tile <= 0) return;

    for (int ty = 0; ty < tiles_h; ++ty) {
        for (int tx = 0; tx < tiles_w; ++tx) {
            if (!world_tile_is_dynamic(tx, ty)) continue;

            gfx_rect tile_rect = { (float)(tx * tile_px), (float)(ty * tile_px), (float)tile_px, (float)tile_px };
            if (!rects_intersect(tile_rect, view->padded_view)) continue;

            uint16_t mask = world_subtile_mask_at(tx, ty);
            if (mask == 0) continue;

            for (int sy = 0; sy < subtiles_per_tile; ++sy) {
                for (int sx = 0; sx < subtiles_per_tile; ++sx) {
                    int bit = sy * subtiles_per_tile + sx;
                    if ((mask & (uint16_t)(1u << bit)) == 0) continue;

                    gfx_rect r = {
                        tile_rect.x + (float)(sx * subtile_px),
                        tile_rect.y + (float)(sy * subtile_px),
                        (float)subtile_px,
                        (float)subtile_px
                    };
                    draw_collider_outline(r, &view->padded_view, color);
                }
            }
        }
    }
}
#endif

void draw_debug_collision_overlays(const render_view_t* view)
{
#if DEBUG_BUILD && DEBUG_COLLISION
    if (!g_draw_ecs_colliders && !g_draw_phys_colliders && !g_draw_static_colliders) return;
    gfx_color ecs_color = GFX_RED;
    gfx_color phys_color = GFX_BLUE;
    gfx_color static_color = GFX_WHITE;

    if (g_draw_static_colliders) {
        draw_static_colliders(view, static_color);
        draw_dynamic_colliders(view, static_color);
    }

    if (g_draw_ecs_colliders || g_draw_phys_colliders) {
        for (ecs_collider_iter_t it = ecs_colliders_begin(); ; ) {
            ecs_collider_view_t c;
            if (!ecs_colliders_next(&it, &c)) break;

            if (g_draw_ecs_colliders) {
                gfx_rect bounds = collider_bounds_at(c.ecs_x, c.ecs_y, c.hx, c.hy);
                draw_collider_outline(bounds, &view->padded_view, ecs_color);
            }

            if (g_draw_phys_colliders && c.has_phys) {
                gfx_rect bounds = collider_bounds_at(c.phys_x, c.phys_y, c.hx, c.hy);
                draw_collider_outline(bounds, &view->padded_view, phys_color);
            }
        }
    }
#else
    (void)view;
#endif
}

void draw_debug_trigger_overlays(const render_view_t* view)
{
#if DEBUG_BUILD && DEBUG_TRIGGERS
    if (!g_draw_triggers) return;
    for (ecs_trigger_iter_t it = ecs_triggers_begin(); ; ) {
        ecs_trigger_view_t c;
        if (!ecs_triggers_next(&it, &c)) break;

        gfx_rect bounds = (gfx_rect){ .x = c.x - c.hx - c.pad, .y = c.y - c.hy - c.pad, .w = 2.f * c.hx + 2.f * c.pad, .h = 2.f * c.hy + 2.f * c.pad  };
        if (!rects_intersect(bounds, view->padded_view)) continue;
        gfx_draw_rect_lines(bounds, GFX_GREEN);
    }
#else
    (void)view;
#endif
}

void renderer_debug_draw_ui(const render_view_t* view)
{
#if DEBUG_BUILD && DEBUG_FPS
    (void)view;
    if (!g_show_fps) return;

    int fps = time_fps();
    float ms = time_frame_dt() * 1000.0f;
    char buf[64];
    snprintf(buf, sizeof(buf), "FPS: %d | %.2f ms", fps, ms);

    int fs = 18;
    int tw = gfx_measure_text(buf, fs);
    int x = (gfx_screen_width() - tw)/2;
    int y = gfx_screen_height() - fs - 6;

    gfx_draw_rect((gfx_rect){ .x = (float)(x - 8), .y = (float)(y - 4), .w = (float)(tw + 16), .h = (float)(fs + 8)  }, GFX_COLOR(0, 0, 0, 160));
    gfx_draw_text(buf, x, y, fs, GFX_RAYWHITE);
#else
    (void)view;
#endif
}
