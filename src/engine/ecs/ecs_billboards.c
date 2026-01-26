//==== FROM ecs_billboards.c ====
#include "engine/ecs/ecs_engine.h"
#include "engine/ecs/ecs_billboards.h"
#include "engine/ecs/ecs_proximity.h"
#include "engine/engine/engine_scheduler/engine_scheduler_registration.h"
#include "shared/utils/dynarray.h"

typedef struct {
    ecs_billboard_filter_fn fn;
    void* data;
} ecs_billboard_filter_entry_t;

static DA(ecs_billboard_filter_entry_t) billboard_filters = {0};

void ecs_billboards_clear_filters(void)
{
    DA_CLEAR(&billboard_filters);
}

void ecs_billboards_add_filter(ecs_billboard_filter_fn fn, void* data)
{
    if (!fn) return;
    DA_APPEND(&billboard_filters, ((ecs_billboard_filter_entry_t){ .fn = fn, .data = data }));
}

static bool billboard_filters_pass(int trigger_idx, int matched_idx)
{
    for (size_t i = 0; i < billboard_filters.size; ++i) {
        ecs_billboard_filter_entry_t* entry = &billboard_filters.data[i];
        if (!entry->fn(trigger_idx, matched_idx, entry->data)) return false;
    }
    return true;
}

static void sys_billboards_impl(float dt)
{
    (void)dt;
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i) || !(ecs_mask[i] & CMP_BILLBOARD)) continue;
        cmp_billboard[i].state = BILLBOARD_INACTIVE;
        cmp_billboard[i].timer = 0.0f;
    }

    ecs_prox_iter_t it = ecs_prox_stay_begin();
    ecs_prox_view_t v;
    while (ecs_prox_stay_next(&it, &v)) {
        int trigger_idx = ent_index_checked(v.trigger_owner);
        int matched_idx = ent_index_checked(v.matched_entity);
        if (trigger_idx < 0 || matched_idx < 0) continue;
        if ((ecs_mask[trigger_idx] & CMP_BILLBOARD) == 0) continue;
        if (!billboard_filters_pass(trigger_idx, matched_idx)) continue;

        cmp_billboard[trigger_idx].state = BILLBOARD_ACTIVE;
        float timer = cmp_billboard[trigger_idx].linger;
        if (timer <= 0.0f) timer = 1.0f;
        cmp_billboard[trigger_idx].timer = timer;
    }
}

SYSTEMS_ADAPT_DT(sys_billboards_adapt, sys_billboards_impl)
