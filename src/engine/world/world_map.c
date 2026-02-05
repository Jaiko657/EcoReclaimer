#include "engine/world/world_map.h"
#include "engine/engine/engine_scheduler/engine_scheduler.h"
#include "engine/world/world_collision_internal.h"
#include "engine/core/logger/logger.h"
#include "engine/utils/dynarray.h"
#include "engine/tiled/tiled.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int layer_idx;
    int tx;
    int ty;
    uint32_t raw_gid;
} world_tile_edit_t;

static world_map_t g_world_map = {0};
static bool g_tiled_ready = false;
static DA(world_tile_edit_t) g_tile_edits = {0};
static uint32_t g_map_gen = 0;
static bool* g_anim_disabled = NULL;
static size_t g_anim_disabled_count = 0;

static void world_anim_disabled_reset(void)
{
    free(g_anim_disabled);
    g_anim_disabled = NULL;
    g_anim_disabled_count = 0;
}

static void world_unload_map(void)
{
    if (g_tiled_ready) {
        tiled_free_map(&g_world_map);
        g_tiled_ready = false;
    }
    world_anim_disabled_reset();
}

bool world_load_from_tmx(const char* tmx_path, const char* collision_layer_name)
{
    world_map_t new_map;
    if (!tiled_load_map(tmx_path, &new_map)) {
        LOGC(LOGCAT_WORLD, LOG_LVL_ERROR, "world: failed to load TMX '%s'", tmx_path ? tmx_path : "(null)");
        return false;
    }

    if (!world_collision_build_from_map(&new_map, collision_layer_name)) {
        tiled_free_map(&new_map);
        return false;
    }

    world_unload_map();
    g_world_map = new_map;
    g_tiled_ready = true;
    g_map_gen++;

    // Drop any pending edits from the previous map.
    DA_CLEAR(&g_tile_edits);
    world_anim_disabled_reset();
    if (g_world_map.layer_count > 0 && g_world_map.width > 0 && g_world_map.height > 0) {
        size_t total = (size_t)g_world_map.layer_count
            * (size_t)g_world_map.width
            * (size_t)g_world_map.height;
        g_anim_disabled = (bool*)calloc(total, sizeof(bool));
        if (!g_anim_disabled) {
            LOGC(LOGCAT_WORLD, LOG_LVL_WARN, "world: out of memory for anim-disable grid (%zu cells)", total);
        } else {
            g_anim_disabled_count = total;
        }
    }

    LOGC(LOGCAT_WORLD, LOG_LVL_INFO, "world: loaded TMX '%s' (%dx%d)", tmx_path, g_world_map.width, g_world_map.height);
    return true;
}

void world_shutdown(void)
{
    DA_FREE(&g_tile_edits);
    world_unload_map();
    world_collision_shutdown();
}

bool world_has_map(void)
{
    return g_tiled_ready;
}

bool world_get_map_info(world_map_info_t* out)
{
    if (!g_tiled_ready) return false;
    if (out) {
        *out = (world_map_info_t){
        .width_tiles = g_world_map.width,
        .height_tiles = g_world_map.height,
        .tile_width = g_world_map.tilewidth,
        .tile_height = g_world_map.tileheight,
        .layer_count = (int)g_world_map.layer_count,
        .object_count = (int)g_world_map.object_count,
        };
    }
    return true;
}

const world_map_t* world_get_map(void)
{
    return g_tiled_ready ? &g_world_map : NULL;
}

uint32_t world_map_generation(void)
{
    return g_map_gen;
}

bool world_set_tile_gid(int layer_idx, int tx, int ty, uint32_t raw_gid)
{
    if (!g_tiled_ready) return false;
    if (layer_idx < 0 || (size_t)layer_idx >= g_world_map.layer_count) return false;
    if (tx < 0 || ty < 0 || tx >= g_world_map.width || ty >= g_world_map.height) return false;

    tiled_layer_t* layer = &g_world_map.layers[(size_t)layer_idx];
    if (tx >= layer->width || ty >= layer->height) return false;
    if (!layer->gids) return false;

    world_tile_edit_t e = { layer_idx, tx, ty, raw_gid };
    DA_APPEND(&g_tile_edits, e);
    return true;
}

static bool anim_disabled_index(int layer_idx, int tx, int ty, size_t* out_idx)
{
    if (!g_tiled_ready) return false;
    if (!g_anim_disabled || g_anim_disabled_count == 0) return false;
    if (layer_idx < 0 || (size_t)layer_idx >= g_world_map.layer_count) return false;
    if (tx < 0 || ty < 0 || tx >= g_world_map.width || ty >= g_world_map.height) return false;

    tiled_layer_t* layer = &g_world_map.layers[(size_t)layer_idx];
    if (tx >= layer->width || ty >= layer->height) return false;
    size_t idx = ((size_t)layer_idx * (size_t)g_world_map.width * (size_t)g_world_map.height)
        + (size_t)ty * (size_t)g_world_map.width
        + (size_t)tx;
    if (idx >= g_anim_disabled_count) return false;
    if (out_idx) *out_idx = idx;
    return true;
}

bool world_tile_anim_disable(int layer_idx, int tx, int ty, bool disable)
{
    size_t idx = 0;
    if (!anim_disabled_index(layer_idx, tx, ty, &idx)) return false;
    g_anim_disabled[idx] = disable;
    return true;
}

bool world_tile_anim_is_disabled(int layer_idx, int tx, int ty)
{
    size_t idx = 0;
    if (!anim_disabled_index(layer_idx, tx, ty, &idx)) return false;
    return g_anim_disabled[idx];
}

void world_apply_tile_edits(void)
{
    if (!g_tiled_ready) {
        DA_CLEAR(&g_tile_edits);
        return;
    }

    for (size_t i = 0; i < g_tile_edits.size; ++i) {
        world_tile_edit_t e = g_tile_edits.data[i];
        if (e.layer_idx < 0 || (size_t)e.layer_idx >= g_world_map.layer_count) continue;
        if (e.tx < 0 || e.ty < 0 || e.tx >= g_world_map.width || e.ty >= g_world_map.height) continue;

        tiled_layer_t* layer = &g_world_map.layers[(size_t)e.layer_idx];
        if (!layer->gids) continue;
        if (e.tx >= layer->width || e.ty >= layer->height) continue;

        const size_t idx = (size_t)e.ty * (size_t)layer->width + (size_t)e.tx;
        layer->gids[idx] = e.raw_gid;

        if (layer->collision) {
            world_collision_refresh_tile(&g_world_map, e.tx, e.ty);
        }
    }

    DA_CLEAR(&g_tile_edits);
}

SYSTEMS_ADAPT_VOID(sys_world_apply_edits_adapt, world_apply_tile_edits)
