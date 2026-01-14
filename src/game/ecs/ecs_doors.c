//==== FROM ecs_doors.c ====
#include "game/ecs/ecs_game.h"
#include "game/ecs/ecs_doors.h"
#include "engine/world/world.h"
#include "engine/world/world_renderer.h"

#include <stdlib.h>

static void door_release_tiles(cmp_door_t* d)
{
    if (!d || !d->tiles || d->tile_count <= 0) return;
    for (int i = 0; i < d->tile_count; ++i) {
        door_tile_info_t* t = &d->tiles[i];
        if (t->layer_idx >= 0) {
            world_tile_anim_disable(t->layer_idx, t->tx, t->ty, false);
        }
    }
    free(d->tiles);
    d->tiles = NULL;
    d->tile_count = 0;
    d->primary_anim_total_ms = 0;
}

static void door_init_tiles(cmp_door_t* d, const door_tile_xy_t* tile_xy, int tile_count)
{
    if (!d || !d->tiles || tile_count <= 0 || !tile_xy) return;
    const world_map_t* map = world_get_map();
    if (!map) return;

    const uint32_t FLIP_MASK = 0xE0000000u;
    const uint32_t GID_MASK = TILED_GID_MASK;

    for (int i = 0; i < tile_count; ++i) {
        door_tile_info_t* t = &d->tiles[i];
        t->tx = tile_xy[i].x;
        t->ty = tile_xy[i].y;
        t->layer_idx = -1;
        t->tileset_idx = -1;
        t->base_tile_id = -1;
        t->flip_flags = 0;

        uint32_t raw_gid = 0;
        for (size_t li = map->layer_count; li-- > 0; ) {
            const tiled_layer_t* layer = &map->layers[li];
            if (!layer->gids) continue;
            if (t->tx < 0 || t->ty < 0 || t->tx >= layer->width || t->ty >= layer->height) continue;
            uint32_t gid = layer->gids[(size_t)t->ty * (size_t)layer->width + (size_t)t->tx];
            if (gid == 0) continue;
            raw_gid = gid;
            t->layer_idx = (int)li;
            break;
        }
        if (raw_gid == 0) continue;

        t->flip_flags = raw_gid & FLIP_MASK;
        uint32_t bare_gid = raw_gid & GID_MASK;
        int chosen_ts = -1;
        int local_id = 0;
        for (size_t si = 0; si < map->tileset_count; ++si) {
            const tiled_tileset_t* ts = &map->tilesets[si];
            int local = (int)bare_gid - ts->first_gid;
            if (local < 0 || local >= ts->tilecount) continue;
            chosen_ts = (int)si;
            local_id = local;
            break;
        }
        t->tileset_idx = chosen_ts;
        t->base_tile_id = local_id;
    }

    d->primary_anim_total_ms = 0;
    for (int i = 0; i < tile_count; ++i) {
        door_tile_info_t* t = &d->tiles[i];
        if (t->tileset_idx < 0 || t->base_tile_id < 0) continue;
        if ((size_t)t->tileset_idx >= map->tileset_count) continue;
        const tiled_tileset_t* ts = &map->tilesets[(size_t)t->tileset_idx];
        if (!ts || !ts->anims) continue;
        if (t->base_tile_id < 0 || t->base_tile_id >= ts->tilecount) continue;
        tiled_animation_t* anim = &ts->anims[t->base_tile_id];
        if (anim && anim->total_duration_ms > 0) {
            d->primary_anim_total_ms = anim->total_duration_ms;
            break;
        }
    }

    for (int i = 0; i < tile_count; ++i) {
        door_tile_info_t* t = &d->tiles[i];
        if (t->layer_idx < 0 || t->tileset_idx < 0 || t->base_tile_id < 0) continue;
        if ((size_t)t->tileset_idx >= map->tileset_count) continue;
        const tiled_tileset_t* ts = &map->tilesets[(size_t)t->tileset_idx];
        world_tile_anim_disable(t->layer_idx, t->tx, t->ty, true);
        uint32_t gid = (uint32_t)(ts->first_gid + t->base_tile_id) | t->flip_flags;
        world_set_tile_gid(t->layer_idx, t->tx, t->ty, gid);
    }
}

void ecs_door_on_destroy(int idx)
{
    if (idx < 0 || idx >= ECS_MAX_ENTITIES) return;
    if (!(ecs_mask[idx] & CMP_DOOR)) return;
    door_release_tiles(&cmp_door[idx]);
}

void cmp_add_door(ecs_entity_t e, float prox_radius, int tile_count, const door_tile_xy_t* tile_xy)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_door_t* d = &cmp_door[i];
    door_release_tiles(d);
    *d = (cmp_door_t){0};
    d->prox_radius = prox_radius;
    if (tile_xy && tile_count > 0) {
        d->tiles = (door_tile_info_t*)calloc((size_t)tile_count, sizeof(*d->tiles));
        if (d->tiles) {
            d->tile_count = tile_count;
            door_init_tiles(d, tile_xy, tile_count);
        }
    }
    d->state = DOOR_CLOSED;
    d->anim_time_ms = 0.0f;
    d->intent_open = false;
    ecs_mask[i] |= CMP_DOOR;
}
