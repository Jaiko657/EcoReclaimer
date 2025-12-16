#include "modules/core/engine_types.h"
#include "modules/ecs/ecs.h"
#include "modules/ecs/ecs_internal.h"
#include "modules/ecs/ecs_proximity.h"
#include "modules/ecs/ecs_game.h"
#include "modules/systems/systems.h"
#include "modules/systems/systems_registration.h"
#include "modules/core/toast.h"
#include "modules/core/logger.h"
#include "modules/tiled/tiled.h"
#include "modules/ecs/ecs_prefab_loading.h"
#include "modules/world/world.h"
#include "modules/world/world_renderer.h"
#include "modules/ecs/ecs_door_systems.h"
#include <stdio.h>

bool init_entities(const char* tmx_path)
{
    const world_map_t* map = world_get_map();
    if (!map) {
        LOGC(LOGCAT_ECS, LOG_LVL_ERROR, "init_entities: no tiled map loaded");
        return false;
    }

    ecs_prefab_spawn_from_map(map, tmx_path);
    return true;
}

// ===== Game-side component storage =====
typedef struct {
    int counts[RESOURCE_TYPE_COUNT];
    int capacity;
} cmp_storage_t;

static cmp_storage_t g_storage[ECS_MAX_ENTITIES];

static const int k_storage_default_capacity = 20;

// ===== Component adders =====
void cmp_add_storage(ecs_entity_t e, int capacity)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    if (capacity <= 0) capacity = k_storage_default_capacity;
    cmp_storage_t storage = { {0}, capacity };
    g_storage[i] = storage;
    ecs_mask[i] |= CMP_STORAGE;
    if (ecs_mask[i] & CMP_PHYS_BODY) {
        cmp_phys_body[i].category_bits |= PHYS_CAT_TARDAS;
    }
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

// ===== Gameplay helpers =====
bool game_get_tardas_storage(int out_counts[RESOURCE_TYPE_COUNT], int* out_capacity)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_STORAGE) == 0) continue;
        if (out_counts) {
            for (int type = 0; type < RESOURCE_TYPE_COUNT; ++type) {
                out_counts[type] = g_storage[i].counts[type];
            }
        }
        if (out_capacity) *out_capacity = g_storage[i].capacity;
        return true;
    }
    if (out_counts) {
        for (int type = 0; type < RESOURCE_TYPE_COUNT; ++type) {
            out_counts[type] = 0;
        }
    }
    if (out_capacity) *out_capacity = 0;
    return false;
}

bool game_get_grav_gun_charge(float* out_charge, float* out_max)
{
    ecs_entity_t player = find_player_handle();
    int player_idx = ent_index_checked(player);
    if (player_idx < 0 || (ecs_mask[player_idx] & CMP_PLAYER) == 0) {
        if (out_charge) *out_charge = 0.0f;
        if (out_max) *out_max = 0.0f;
        return false;
    }

    ecs_entity_t gun = cmp_player[player_idx].held_gun;
    int gun_idx = ent_index_checked(gun);
    if (gun_idx < 0 || (ecs_mask[gun_idx] & CMP_GRAV_GUN) == 0) {
        cmp_player[player_idx].held_gun = ecs_null();
        if (out_charge) *out_charge = 0.0f;
        if (out_max) *out_max = 0.0f;
        return false;
    }
    if (!cmp_grav_gun[gun_idx].held ||
        cmp_grav_gun[gun_idx].holder.idx != player.idx ||
        cmp_grav_gun[gun_idx].holder.gen != player.gen) {
        cmp_player[player_idx].held_gun = ecs_null();
        if (out_charge) *out_charge = 0.0f;
        if (out_max) *out_max = 0.0f;
        return false;
    }

    if (out_charge) *out_charge = cmp_grav_gun[gun_idx].charge;
    if (out_max) *out_max = cmp_grav_gun[gun_idx].max_charge;
    return true;
}

bool ecs_game_get_storage(ecs_entity_t e, int out_counts[RESOURCE_TYPE_COUNT], int* out_capacity)
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

// ===== Systems: storage =====
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

// ===== System adapters + registration =====
SYSTEMS_ADAPT_VOID(sys_storage_deposit_adapt, sys_storage_deposit_impl)

void ecs_register_game_systems(void)
{
    // maintain original ordering around billboards
    systems_register(PHASE_SIM_POST, 120, sys_storage_deposit_adapt, "storage_deposit");
    ecs_register_door_systems();
}
