#include "game/ecs/helpers/ecs_storage_helpers.h"

#include "game/ecs/helpers/ecs_player_helpers.h"
#include "engine/ecs/ecs_core.h"
#include "engine/core/time/time.h" //for random seed

#include <limits.h>
#include <stdint.h>

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
    cmp_storage[i] = (cmp_storage_t){ .counts = {0}, .capacity = capacity };
    ecs_mask[i] |= CMP_STORAGE;
}

bool ecs_storage_get(ecs_entity_t e, int out_counts[RESOURCE_TYPE_COUNT], int* out_capacity)
{
    int i = ent_index_checked(e);
    if (i < 0) return false;
    if ((ecs_mask[i] & CMP_STORAGE) == 0) return false;
    if (out_counts) {
        for (int type = 0; type < RESOURCE_TYPE_COUNT; ++type) {
            out_counts[type] = cmp_storage[i].counts[type];
        }
    }
    if (out_capacity) *out_capacity = cmp_storage[i].capacity;
    return true;
}

bool ecs_storage_add_resource(ecs_entity_t e, resource_type_t type, int count)
{
    int i = ent_index_checked(e);
    if (i < 0) return false;
    if ((ecs_mask[i] & CMP_STORAGE) == 0) return false;
    if (count <= 0) return false;
    cmp_storage[i].counts[type] += count;
    return true;
}

bool ecs_storage_take_random(ecs_entity_t e, resource_type_t* out_type)
{
    int i = ent_index_checked(e);
    if (i < 0) return false;
    if ((ecs_mask[i] & CMP_STORAGE) == 0) return false;

    int total = storage_total(&cmp_storage[i]);
    if (total <= 0) return false;

    int pick = (int)(storage_rng_next() % (uint32_t)total);
    for (int type = 0; type < RESOURCE_TYPE_COUNT; ++type) {
        int count = cmp_storage[i].counts[type];
        if (count <= 0) continue;
        if (pick < count) {
            cmp_storage[i].counts[type] -= 1;
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
