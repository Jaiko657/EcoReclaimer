#include "modules/ecs/ecs_recycler.h"
#include "modules/ecs/ecs_internal.h"
#include "modules/ecs/ecs_physics.h"
#include "modules/ecs/ecs_proximity.h"
#include "modules/ecs/ecs_storage.h"
#include "modules/systems/systems.h"
#include "modules/systems/systems_registration.h"
#include "modules/core/toast.h"

typedef struct {
    resource_type_t type;
    ecs_entity_t storage;
} cmp_recycle_bin_t;

static cmp_recycle_bin_t g_recycle_bin[ECS_MAX_ENTITIES];

static const float k_recycle_fall_speed = 50.0f;

void cmp_add_recycle_bin(ecs_entity_t e, resource_type_t type)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    g_recycle_bin[i] = (cmp_recycle_bin_t){ type, ecs_null() };
    ecs_mask[i] |= CMP_RECYCLE_BIN;
}

static void sys_recycle_bins_impl(void)
{
    ecs_prox_iter_t it = ecs_prox_stay_begin();
    ecs_prox_view_t v;
    while (ecs_prox_stay_next(&it, &v)) {
        int ia = ent_index_checked(v.trigger_owner);
        int ib = ent_index_checked(v.matched_entity);
        if (ia < 0 || ib < 0) continue;

        if ((ecs_mask[ia] & CMP_RECYCLE_BIN) == 0) continue;
        if ((ecs_mask[ib] & (CMP_RESOURCE | CMP_LIFTABLE)) != (CMP_RESOURCE | CMP_LIFTABLE)) continue;

        cmp_liftable_t* g = &cmp_liftable[ib];
        if (g->state == GRAV_GUN_STATE_HELD) continue;
        if (g->recycle_active) continue;
        if (!g->just_dropped) continue;

        cmp_recycle_bin_t* bin = &g_recycle_bin[ia];
        resource_type_t dropped_type = cmp_resource_type_from_index(ib);
        if (dropped_type != bin->type) {
            ui_toast(1.0f, "Incorrect material");
            continue;
        }

        ecs_entity_t storage_entity = bin->storage;
        int storage_idx = ent_index_checked(storage_entity);
        if (storage_idx < 0 || (ecs_mask[storage_idx] & CMP_STORAGE) == 0) {
            storage_entity = ecs_storage_find_player();
            storage_idx = ent_index_checked(storage_entity);
            if (storage_idx < 0 || (ecs_mask[storage_idx] & CMP_STORAGE) == 0) continue;
            bin->storage = storage_entity;
        }

        ecs_storage_add_resource(storage_entity, dropped_type, 1);

        float bin_x = cmp_pos[ia].x;
        float bin_y = cmp_pos[ia].y;
        float bin_hy = (ecs_mask[ia] & CMP_COL) ? cmp_col[ia].hy : 0.0f;
        float res_hy = (ecs_mask[ib] & CMP_COL) ? cmp_col[ib].hy : 0.0f;
        float top = bin_y - bin_hy + res_hy;
        float bottom = bin_y + bin_hy - res_hy;
        if (bottom < top) bottom = top;

        cmp_pos[ib].x = bin_x;
        cmp_pos[ib].y = top;

        if (ecs_mask[ib] & CMP_PHYS_BODY) {
            ecs_phys_body_destroy_for_entity(ib);
            ecs_mask[ib] &= ~CMP_PHYS_BODY;
        }
        if (ecs_mask[ib] & CMP_VEL) {
            cmp_vel[ib].x = 0.0f;
            cmp_vel[ib].y = 0.0f;
        }

        g->recycle_active = true;
        g->recycle_target_y = bottom;
        g->just_dropped = false;

        ui_toast(1.0f, "%s recycled", resource_type_to_string(dropped_type));
    }
}

static void sys_recycle_anim_impl(float dt)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & (CMP_LIFTABLE | CMP_POS)) != (CMP_LIFTABLE | CMP_POS)) continue;
        cmp_liftable_t* g = &cmp_liftable[i];
        if (!g->recycle_active) continue;

        cmp_pos[i].y += k_recycle_fall_speed * dt;
        if (cmp_pos[i].y >= g->recycle_target_y) {
            ecs_destroy(handle_from_index(i));
        }
    }
}

SYSTEMS_ADAPT_VOID(sys_recycle_bins_adapt, sys_recycle_bins_impl)
SYSTEMS_ADAPT_DT(sys_recycle_anim_adapt, sys_recycle_anim_impl)
