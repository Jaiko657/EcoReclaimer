#include "engine/renderer/renderer.h"
#include "engine/renderer/renderer_internal.h"
#include "engine/core/logger/logger.h"
#include "engine/runtime/camera.h"
#include "engine/core/platform/platform.h"
#include "engine/world/world_map.h"
#include "engine/world/world_query.h"

static renderer_ctx_t g_renderer = {0};
static platform_window* g_window = NULL;

renderer_ctx_t* renderer_ctx_get(void)
{
    return &g_renderer;
}

bool renderer_init(int width, int height, const char* title, int target_fps)
{
    g_window = platform_window_create(width, height, title, target_fps);
    if (!g_window) {
        LOGC(LOGCAT_REND, LOG_LVL_FATAL, "Renderer: window failed to init");
        return false;
    }

    if (!gfx_init_renderer(g_window)) {
        platform_window_destroy(g_window);
        g_window = NULL;
        return false;
    }
    return true;
}

bool renderer_bind_world_map(void)
{
    renderer_ctx_t* ctx = renderer_ctx_get();
    const world_map_t* map = world_get_map();
    if (!map) {
        LOGC(LOGCAT_REND, LOG_LVL_ERROR, "tiled: no world map loaded");
        return false;
    }

    tiled_renderer_t new_tiled_renderer;
    if (!tiled_renderer_init(&new_tiled_renderer, map)) {
        LOGC(LOGCAT_REND, LOG_LVL_ERROR, "tiled: renderer init failed (world map)");
        return false;
    }

    int world_w = 0, world_h = 0;
    if (world_size_tiles(&world_w, &world_h)) {
        if (world_w > 0 && world_h > 0 && (world_w != map->width || world_h != map->height)) {
            LOGC(LOGCAT_REND, LOG_LVL_WARN, "tiled: TMX size %dx%d differs from collision map %dx%d", map->width, map->height, world_w, world_h);
        }
    } else {
        LOGC(LOGCAT_REND, LOG_LVL_WARN, "tiled: collision map size unavailable");
    }
    int tw = world_tile_size();
    if (tw > 0 && tw != map->tilewidth) {
        LOGC(LOGCAT_REND, LOG_LVL_WARN, "tiled: TMX tilewidth %d differs from engine tile size %d", map->tilewidth, tw);
    }

    renderer_unload_tiled_map();
    ctx->tiled = new_tiled_renderer;
    ctx->bound_gen = world_map_generation();

    LOGC(LOGCAT_REND, LOG_LVL_INFO, "tiled: bound world map (%dx%d @ %dx%d)", map->width, map->height, map->tilewidth, map->tileheight);
    return true;
}

void renderer_unload_tiled_map(void)
{
    renderer_ctx_t* ctx = renderer_ctx_get();
    if (ctx->bound_gen == 0) return;
    tiled_renderer_shutdown(&ctx->tiled);
    ctx->bound_gen = 0;
}

void renderer_shutdown(void)
{
    renderer_ctx_t* ctx = renderer_ctx_get();
    renderer_unload_tiled_map();
    DA_FREE(&ctx->painter_items);
    gfx_shutdown();
    if (g_window) {
        platform_window_destroy(g_window);
        g_window = NULL;
    }
}

bool renderer_screen_to_world(float screen_x, float screen_y, float* out_x, float* out_y)
{
    if (!out_x || !out_y) return false;

    camera_view_t logical = camera_get_view();
    int sw = gfx_screen_width();
    int sh = gfx_screen_height();
    if (sw <= 0 || sh <= 0) {
        *out_x = logical.center.x;
        *out_y = logical.center.y;
        return false;
    }

    gfx_camera2d cam = {
        .target   = (gfx_vec2){ .x = logical.center.x, .y = logical.center.y  },
        .offset   = (gfx_vec2){ .x = sw / 2.0f, .y = sh / 2.0f  },
        .rotation = 0.0f,
        .zoom     = logical.zoom > 0.0f ? logical.zoom : 1.0f
    };

    gfx_vec2 world = gfx_screen_to_world((gfx_vec2){ .x = screen_x, .y = screen_y  }, &cam);
    *out_x = world.x;
    *out_y = world.y;
    return true;
}
