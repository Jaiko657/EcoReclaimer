//==== FROM ecs_unloader.c ====
#include "game/ecs/ecs_game.h"
#include "engine/prefab/loading/pf_loading.h"
#include "engine/ecs/ecs_proximity.h"
#include "game/ecs/helpers/ecs_storage_helpers.h"
#include "engine/engine/engine_scheduler/engine_scheduler.h"
#include "engine/engine/engine_scheduler/engine_scheduler_registration.h"

#include <float.h>

static ecs_entity_t find_nearest_unpacker(int unloader_idx)
{
    ecs_entity_t best = ecs_null();
    float best_dist = FLT_MAX;

    const bool has_pos = (ecs_mask[unloader_idx] & CMP_POS) != 0;
    const float ux = has_pos ? cmp_pos[unloader_idx].x : 0.0f;
    const float uy = has_pos ? cmp_pos[unloader_idx].y : 0.0f;

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & (CMP_UNPACKER | CMP_POS)) != (CMP_UNPACKER | CMP_POS)) continue;
        float dx = cmp_pos[i].x - ux;
        float dy = cmp_pos[i].y - uy;
        float d2 = dx * dx + dy * dy;
        if (d2 < best_dist) {
            best_dist = d2;
            best = handle_from_index(i);
        }
    }

    return best;
}

static void unpacker_refresh_ready_state(int idx)
{
    if ((ecs_mask[idx] & CMP_UNPACKER) == 0) return;
    cmp_unpacker_t* u = &cmp_unpacker[idx];
    if (!u->ready && !ecs_alive_handle(u->spawned_entity)) {
        u->ready = true;
        u->spawned_entity = ecs_null();
    }
}

static void sys_unloader_tick_impl(void)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_UNPACKER) == 0) continue;
        unpacker_refresh_ready_state(i);
    }

    bool has_target[ECS_MAX_ENTITIES] = {0};
    ecs_entity_t target[ECS_MAX_ENTITIES];
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        target[i] = ecs_null();
    }

    ecs_prox_iter_t it = ecs_prox_stay_begin();
    ecs_prox_view_t v;
    while (ecs_prox_stay_next(&it, &v)) {
        int ia = ent_index_checked(v.trigger_owner);
        int ib = ent_index_checked(v.matched_entity);
        if (ia < 0 || ib < 0) continue;
        if ((ecs_mask[ia] & CMP_UNLOADER) == 0) continue;
        if ((ecs_mask[ib] & (CMP_STORAGE | CMP_LIFTABLE)) != (CMP_STORAGE | CMP_LIFTABLE)) continue;

        if (!has_target[ia]) {
            has_target[ia] = true;
            target[ia] = v.matched_entity;
        }
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_UNLOADER) == 0) continue;
        if (!has_target[i]) continue;

        cmp_unloader_t* unloader = &cmp_unloader[i];

        ecs_entity_t unpacker = unloader->unpacker_handle;
        int unpacker_idx = ent_index_checked(unpacker);
        if (unpacker_idx < 0 || (ecs_mask[unpacker_idx] & CMP_UNPACKER) == 0) {
            unpacker = find_nearest_unpacker(i);
            unloader->unpacker_handle = unpacker;
            unpacker_idx = ent_index_checked(unpacker);
        }
        if (unpacker_idx < 0 || (ecs_mask[unpacker_idx] & CMP_UNPACKER) == 0) continue;
        unpacker_refresh_ready_state(unpacker_idx);
        if (!cmp_unpacker[unpacker_idx].ready) continue;

        resource_type_t type;
        if (!ecs_storage_take_random(target[i], &type)) {
            continue;
        }

        float spawn_x = (ecs_mask[i] & CMP_POS) ? cmp_pos[i].x : 0.0f;
        float spawn_y = (ecs_mask[i] & CMP_POS) ? cmp_pos[i].y : 0.0f;
        if ((ecs_mask[unpacker_idx] & CMP_POS) != 0) {
            spawn_x = cmp_pos[unpacker_idx].x;
            spawn_y = cmp_pos[unpacker_idx].y;
        }

        ecs_entity_t spawned = pf_spawn_entity_from_path(
            type == RESOURCE_TYPE_METAL ? "assets/prefabs/metal.ent" : "assets/prefabs/plastic.ent",
            NULL);
        int spawned_idx = ent_index_checked(spawned);
        if (spawned_idx < 0) continue;
        cmp_add_position(spawned, spawn_x, spawn_y);

        cmp_unpacker[unpacker_idx].ready = false;
        cmp_unpacker[unpacker_idx].spawned_entity = spawned;
    }
}

SYSTEMS_ADAPT_VOID(sys_unloader_tick_adapt, sys_unloader_tick_impl)
