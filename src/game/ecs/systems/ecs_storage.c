//==== FROM ecs_storage.c ====
#include "game/ecs/helpers/ecs_storage_helpers.h"
#include "game/ecs/ecs_game.h"
#include "game/ecs/helpers/ecs_resource_helpers.h"
#include "engine/ecs/ecs_proximity.h"
#include "engine/engine/engine_scheduler/engine_scheduler.h"
#include "engine/engine/engine_scheduler/engine_scheduler_registration.h"
#include "engine/runtime/toast.h"
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

        int counts[RESOURCE_TYPE_COUNT];
        int capacity = 0;
        if (!ecs_storage_get(v.trigger_owner, counts, &capacity)) continue;

        int total = 0;
        for (int type = 0; type < RESOURCE_TYPE_COUNT; ++type) {
            total += counts[type];
        }

        if (total >= capacity) {
            ui_toast(1.0f, "TARDAS full (%d/%d)", total, capacity);
            continue;
        }

        resource_type_t type = cmp_resource_type_from_index(ib);
        ecs_storage_add_resource(v.trigger_owner, type, 1);
        int new_total = total + 1;
        const char* type_name = resource_type_to_string(type);
        ecs_destroy(v.matched_entity);
        ui_toast(1.0f, "%s stored (%d/%d)", type_name, new_total, capacity);
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_LIFTABLE) == 0) continue;
        cmp_liftable[i].just_dropped = false;
    }
}

SYSTEMS_ADAPT_VOID(sys_storage_deposit_adapt, sys_storage_deposit_impl)
