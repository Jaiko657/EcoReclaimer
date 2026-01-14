//==== FROM ecs_door_systems.c ====
#include "game/ecs/ecs_game.h"
#include "game/ecs/ecs_proximity.h"
#include "engine/input/input.h"
#include "shared/buttons.h"
#include "engine/systems/systems.h"
#include "engine/systems/systems_registration.h"
#include "engine/world/world.h"
#include "engine/world/world_renderer.h"

static void sys_doors_tick(float dt);

static int door_frame_at(const tiled_tileset_t* ts, int base_tile, float t_ms, bool opening)
{
    if (!ts || base_tile < 0 || base_tile >= ts->tilecount) return base_tile;
    tiled_animation_t* anim = ts->anims ? &ts->anims[base_tile] : NULL;
    if (!anim || anim->frame_count == 0 || anim->total_duration_ms <= 0) return base_tile;

    int frame = base_tile;
    if (opening) {
        int acc = 0;
        for (size_t i = 0; i < anim->frame_count; ++i) {
            acc += anim->frames[i].duration_ms;
            if (t_ms < acc) { frame = anim->frames[i].tile_id; break; }
        }
        if (t_ms >= anim->total_duration_ms) frame = anim->frames[anim->frame_count - 1].tile_id;
    } else {
        int acc = 0;
        for (size_t i = anim->frame_count; i-- > 0; ) {
            acc += anim->frames[i].duration_ms;
            if (t_ms < acc) { frame = anim->frames[i].tile_id; break; }
        }
        if (t_ms >= anim->total_duration_ms) frame = anim->frames[0].tile_id;
    }
    return frame;
}

static void door_apply_state(const cmp_door_t* d, float t_ms, bool play_forward)
{
    if (!d || !d->tiles || d->tile_count <= 0) return;
    const world_map_t* map = world_get_map();
    if (!map) return;

    for (int i = 0; i < d->tile_count; ++i) {
        const door_tile_info_t* t = &d->tiles[i];
        if (t->layer_idx < 0 || t->tileset_idx < 0 || t->base_tile_id < 0) continue;
        if ((size_t)t->layer_idx >= map->layer_count) continue;
        if ((size_t)t->tileset_idx >= map->tileset_count) continue;
        const tiled_tileset_t* ts = &map->tilesets[(size_t)t->tileset_idx];
        if (!ts) continue;
        int frame_tile = door_frame_at(ts, t->base_tile_id, t_ms, play_forward);
        uint32_t gid = (uint32_t)(ts->first_gid + frame_tile) | t->flip_flags;
        world_set_tile_gid(t->layer_idx, t->tx, t->ty, gid);
    }
}

static void sys_doors_tick(float dt)
{
    if (!world_has_map()) return;

    // Build intent from proximity stay/enter
    bool door_should_open[ECS_MAX_ENTITIES] = {0};
    ecs_prox_iter_t stay_it = ecs_prox_stay_begin();
    ecs_prox_view_t v;
    while (ecs_prox_stay_next(&stay_it, &v)) {
        int a = ent_index_checked(v.trigger_owner);
        if (a >= 0 && (ecs_mask[a] & CMP_DOOR)) {
            door_should_open[a] = true;
        }
    }
    ecs_prox_iter_t enter_it = ecs_prox_enter_begin();
    while (ecs_prox_enter_next(&enter_it, &v)) {
        int a = ent_index_checked(v.trigger_owner);
        if (a >= 0 && (ecs_mask[a] & CMP_DOOR)) {
            door_should_open[a] = true;
        }
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_DOOR) != CMP_DOOR) continue;
        cmp_door_t *d = &cmp_door[i];
        d->intent_open = door_should_open[i];
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_DOOR) != CMP_DOOR) continue;
        cmp_door_t *d = &cmp_door[i];

        int primary_total = d->primary_anim_total_ms;
        float prev_t = d->anim_time_ms;
        float dt_ms = dt * 1000.0f;
        if (d->intent_open) {
            if (d->state == DOOR_CLOSED) {
                d->anim_time_ms = 0.0f;
                d->state = DOOR_OPENING;
            } else if (d->state == DOOR_CLOSING) {
                if (primary_total > 0) {
                    float clamped = prev_t;
                    if (clamped > (float)primary_total) clamped = (float)primary_total;
                    d->anim_time_ms = (float)primary_total - clamped;
                } else {
                    d->anim_time_ms = 0.0f;
                }
                d->state = DOOR_OPENING;
            }
        } else {
            if (d->state == DOOR_OPEN) {
                d->anim_time_ms = 0.0f;
                d->state = DOOR_CLOSING;
            } else if (d->state == DOOR_OPENING) {
                if (primary_total > 0) {
                    float clamped = prev_t;
                    if (clamped > (float)primary_total) clamped = (float)primary_total;
                    d->anim_time_ms = (float)primary_total - clamped;
                } else {
                    d->anim_time_ms = 0.0f;
                }
                d->state = DOOR_CLOSING;
            }
        }

        if (d->state == DOOR_OPENING || d->state == DOOR_CLOSING) {
            d->anim_time_ms += dt_ms;
        }

        float t_ms = d->anim_time_ms;
        bool play_forward = true;
        switch (d->state) {
            case DOOR_OPENING:
                play_forward = true;
                break;
            case DOOR_OPEN:
                play_forward = true;
                if (primary_total > 0) t_ms = (float)primary_total;
                break;
            case DOOR_CLOSING:
                play_forward = false;
                break;
            case DOOR_CLOSED:
            default:
                play_forward = true;
                t_ms = 0.0f;
                break;
        }
        if (primary_total > 0 && t_ms > (float)primary_total) {
            t_ms = (float)primary_total;
        }

        door_apply_state(d, t_ms, play_forward);

        if (d->state == DOOR_OPENING) {
            if (primary_total == 0 || d->anim_time_ms >= (float)primary_total) {
                d->state = DOOR_OPEN;
                d->anim_time_ms = (float)primary_total;
            }
        } else if (d->state == DOOR_CLOSING) {
            if (primary_total == 0 || d->anim_time_ms >= (float)primary_total) {
                d->state = DOOR_CLOSED;
                d->anim_time_ms = 0.0f;
            }
        }
    }
}

SYSTEMS_ADAPT_DT(sys_doors_tick_adapt, sys_doors_tick)
