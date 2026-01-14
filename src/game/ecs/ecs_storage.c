//==== FROM ecs_storage.c ====
#include "game/ecs/ecs_storage.h"
#include "game/ecs/ecs_game.h"
#include "game/ecs/ecs_proximity.h"
#include "engine/systems/systems.h"
#include "engine/systems/systems_registration.h"
#include "engine/core/toast.h"
#include "engine/core/time.h"
#include <limits.h>
#include <stdint.h>

typedef struct {
    int counts[RESOURCE_TYPE_COUNT];
    int capacity;
} cmp_storage_t;

static cmp_storage_t g_storage[ECS_MAX_ENTITIES];

static const int k_storage_default_capacity = INT_MAX;
static uint32_t g_storage_rng = 0u;

static uint32_t storage_rng_next(void)
{
    if (g_storage_rng == 0u) {
        double t = time_now();
        uint32_t seed = (uint32_t)(t * 1000.0);
        if (seed == 0u) seed = 1u;
        g_storage_rng = seed;
    }
    g_storage_rng = g_storage_rng * 1664525u + 1013904223u;
    return g_storage_rng;
}

static int storage_total(const cmp_storage_t* storage)
{
    int total = 0;
    if (!storage) return 0;
    for (int i = 0; i < RESOURCE_TYPE_COUNT; ++i) {
        total += storage->counts[i];
    }
    return total;
}

void cmp_add_storage(ecs_entity_t e, int capacity)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    if (capacity <= 0) capacity = k_storage_default_capacity;
    g_storage[i] = (cmp_storage_t){ {0}, capacity };
    ecs_mask[i] |= CMP_STORAGE;
}

bool ecs_storage_get(ecs_entity_t e, int out_counts[RESOURCE_TYPE_COUNT], int* out_capacity)
{
    int i = ent_index_checked(e);
    if (i < 0) return false;
    if ((ecs_mask[i] & CMP_STORAGE) == 0) return false;
    if (out_counts) {
        for (int type = 0; type < RESOURCE_TYPE_COUNT; ++type) {
            out_counts[type] = g_storage[i].counts[type];
        }
    }
    if (out_capacity) *out_capacity = g_storage[i].capacity;
    return true;
}

bool ecs_storage_add_resource(ecs_entity_t e, resource_type_t type, int count)
{
    int i = ent_index_checked(e);
    if (i < 0) return false;
    if ((ecs_mask[i] & CMP_STORAGE) == 0) return false;
    if (count <= 0) return false;
    g_storage[i].counts[type] += count;
    return true;
}

bool ecs_storage_take_random(ecs_entity_t e, resource_type_t* out_type)
{
    int i = ent_index_checked(e);
    if (i < 0) return false;
    if ((ecs_mask[i] & CMP_STORAGE) == 0) return false;

    int total = storage_total(&g_storage[i]);
    if (total <= 0) return false;

    int pick = (int)(storage_rng_next() % (uint32_t)total);
    for (int type = 0; type < RESOURCE_TYPE_COUNT; ++type) {
        int count = g_storage[i].counts[type];
        if (count <= 0) continue;
        if (pick < count) {
            g_storage[i].counts[type] -= 1;
            if (out_type) *out_type = (resource_type_t)type;
            return true;
        }
        pick -= count;
    }

    return false;
}

ecs_entity_t ecs_storage_find_player(void)
{
    ecs_entity_t player = ecs_find_player();
    int idx = ent_index_checked(player);
    if (idx < 0) return ecs_null();
    if ((ecs_mask[idx] & CMP_STORAGE) == 0) return ecs_null();
    return player;
}

ecs_entity_t ecs_storage_find_tardas(void)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & (CMP_STORAGE | CMP_LIFTABLE)) != (CMP_STORAGE | CMP_LIFTABLE)) continue;
        return handle_from_index(i);
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_STORAGE) == 0) continue;
        if (ecs_mask[i] & CMP_PLAYER) continue;
        return handle_from_index(i);
    }
    return ecs_null();
}

static void sys_storage_deposit_impl(void)
{
    ecs_prox_iter_t it = ecs_prox_stay_begin();
    ecs_prox_view_t v;
    while (ecs_prox_stay_next(&it, &v)) {
        int ia = ent_index_checked(v.trigger_owner);
        int ib = ent_index_checked(v.matched_entity);
        if (ia < 0 || ib < 0) continue;

        if ((ecs_mask[ia] & CMP_STORAGE) == 0) continue;
        if ((ecs_mask[ib] & (CMP_RESOURCE | CMP_LIFTABLE)) != (CMP_RESOURCE | CMP_LIFTABLE)) continue;

        cmp_liftable_t* g = &cmp_liftable[ib];
        if (g->state == GRAV_GUN_STATE_HELD) continue;
        if (!g->just_dropped) continue;

        resource_type_t type = cmp_resource_type_from_index(ib);
        cmp_storage_t* storage = &g_storage[ia];
        int total = storage_total(storage);
        if (total >= storage->capacity) {
            ui_toast(1.0f, "TARDAS full (%d/%d)", total, storage->capacity);
            continue;
        }

        storage->counts[type] += 1;
        int new_total = storage_total(storage);
        const char* type_name = resource_type_to_string(type);
        ecs_destroy(v.matched_entity);
        ui_toast(1.0f, "%s stored (%d/%d)", type_name, new_total, storage->capacity);
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_LIFTABLE) == 0) continue;
        cmp_liftable[i].just_dropped = false;
    }
}

SYSTEMS_ADAPT_VOID(sys_storage_deposit_adapt, sys_storage_deposit_impl)
