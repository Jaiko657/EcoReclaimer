#include "modules/ecs/ecs_internal.h"
#include "modules/core/input.h"
#include "modules/systems/systems_registration.h"
#include "modules/ecs/ecs_proximity.h"
#include "modules/common/dynarray.h"
#include <math.h>
#include <string.h>

// =============== Proximity View (transient each tick) =============
static DA(ecs_prox_view_t) prox_curr = {0};
static DA(ecs_prox_view_t) prox_prev = {0};

static bool prox_contains(const ecs_prox_view_t* arr, int n, ecs_prox_view_t p){
    for (int i = 0; i < n; ++i){
        if (arr[i].trigger_owner.idx==p.trigger_owner.idx && arr[i].trigger_owner.gen==p.trigger_owner.gen &&
            arr[i].matched_entity.idx==p.matched_entity.idx && arr[i].matched_entity.gen==p.matched_entity.gen) return true;
    }
    return false;
}

// public iterators
ecs_prox_iter_t ecs_prox_stay_begin(void){ return (ecs_prox_iter_t){ .i = -1 }; }

bool ecs_prox_stay_next(ecs_prox_iter_t* it, ecs_prox_view_t* out){
    int count = (int)prox_curr.size;
    int i = it->i + 1;
    while (i < count){
        if (ecs_alive_handle(prox_curr.data[i].trigger_owner) && ecs_alive_handle(prox_curr.data[i].matched_entity)){
            it->i = i;
            *out = prox_curr.data[i];
            return true;
        }
        ++i;
    }
    return false;
}

ecs_prox_iter_t ecs_prox_enter_begin(void){ return (ecs_prox_iter_t){ .i = -1 }; }

bool ecs_prox_enter_next(ecs_prox_iter_t* it, ecs_prox_view_t* out){
    int curr_count = (int)prox_curr.size;
    int prev_count = (int)prox_prev.size;
    for (int i = it->i + 1; i < curr_count; ++i){
        if (!ecs_alive_handle(prox_curr.data[i].trigger_owner) || !ecs_alive_handle(prox_curr.data[i].matched_entity)) continue;
        if (!prox_contains(prox_prev.data, prev_count, prox_curr.data[i])){
            it->i = i;
            *out = prox_curr.data[i];
            return true;
        }
    }
    return false;
}

ecs_prox_iter_t ecs_prox_exit_begin(void){ return (ecs_prox_iter_t){ .i = -1 }; }

bool ecs_prox_exit_next(ecs_prox_iter_t* it, ecs_prox_view_t* out){
    int prev_count = (int)prox_prev.size;
    int curr_count = (int)prox_curr.size;
    for (int i = it->i + 1; i < prev_count; ++i){
        if (!ecs_alive_handle(prox_prev.data[i].trigger_owner) || !ecs_alive_handle(prox_prev.data[i].matched_entity)) continue;
        if (!prox_contains(prox_curr.data, curr_count, prox_prev.data[i])){
            it->i = i;
            *out = prox_prev.data[i];
            return true;
        }
    }
    return false;
}

static bool col_overlap_padded_idx(int a, int b, float pad){
    float ax = cmp_pos[a].x, ay = cmp_pos[a].y;
    float bx = cmp_pos[b].x, by = cmp_pos[b].y;

    float ahx = (ecs_mask[a] & CMP_COL) ? (cmp_col[a].hx + pad) : pad;
    float ahy = (ecs_mask[a] & CMP_COL) ? (cmp_col[a].hy + pad) : pad;
    float bhx = (ecs_mask[b] & CMP_COL) ?  cmp_col[b].hx : 0.f;
    float bhy = (ecs_mask[b] & CMP_COL) ?  cmp_col[b].hy : 0.f;

    return fabsf(ax - bx) <= (ahx + bhx) && fabsf(ay - by) <= (ahy + bhy);
}

// ---- systems ----
static void sys_proximity_build_view_impl(void)
{
    if (prox_curr.size > 0) {
        DA_RESERVE(&prox_prev, prox_curr.size);
        memcpy(prox_prev.data, prox_curr.data, sizeof(ecs_prox_view_t) * prox_curr.size);
    }
    prox_prev.size = prox_curr.size;
    DA_CLEAR(&prox_curr);

    for (int a=0; a<ECS_MAX_ENTITIES; ++a){
        if(!ecs_alive_idx(a)) continue;
        if ((ecs_mask[a] & (CMP_POS|CMP_COL|CMP_TRIGGER)) != (CMP_POS|CMP_COL|CMP_TRIGGER)) continue;

        const cmp_trigger_t* tr = &cmp_trigger[a];

        for (int b=0; b<ECS_MAX_ENTITIES; ++b) {
            if (b==a || !ecs_alive_idx(b)) continue;
            if (tr->target_mask) {
                bool matches = false;
                switch (tr->match) {
                    case TRIGGER_MATCH_ANY:
                        matches = (ecs_mask[b] & tr->target_mask) != 0;
                        break;
                    case TRIGGER_MATCH_ALL:
                    default:
                        matches = (ecs_mask[b] & tr->target_mask) == tr->target_mask;
                        break;
                }
                if (!matches) continue;
            }
            if ((ecs_mask[b] & (CMP_POS|CMP_COL)) != (CMP_POS|CMP_COL)) continue;

            if (col_overlap_padded_idx(a, b, tr->pad)){
                ecs_prox_view_t v = { handle_from_index(a), handle_from_index(b) };
                DA_APPEND(&prox_curr, v);
            }
        }
    }
}

static bool resource_held_for_storage(int idx)
{
    if ((ecs_mask[idx] & (CMP_RESOURCE | CMP_LIFTABLE)) != (CMP_RESOURCE | CMP_LIFTABLE)) return false;
    return cmp_liftable[idx].state == GRAV_GUN_STATE_HELD;
}

static bool player_has_grav_gun(ecs_entity_t player)
{
    int player_idx = ent_index_checked(player);
    if (player_idx < 0 || (ecs_mask[player_idx] & CMP_PLAYER) == 0) return false;

    ecs_entity_t gun = cmp_player[player_idx].held_gun;
    int gun_idx = ent_index_checked(gun);
    if (gun_idx < 0 || (ecs_mask[gun_idx] & CMP_GRAV_GUN) == 0) {
        cmp_player[player_idx].held_gun = ecs_null();
        return false;
    }
    if (!cmp_grav_gun[gun_idx].held ||
        cmp_grav_gun[gun_idx].holder.idx != player.idx ||
        cmp_grav_gun[gun_idx].holder.gen != player.gen) {
        cmp_player[player_idx].held_gun = ecs_null();
        return false;
    }
    return true;
}

static bool charger_available(int idx)
{
    if ((ecs_mask[idx] & CMP_GUN_CHARGER) == 0) return false;
    ecs_entity_t stored = cmp_gun_charger[idx].stored_gun;
    return !ecs_alive_handle(stored);
}

static bool billboard_should_activate(int trigger_idx, int matched_idx)
{
    if (trigger_idx < 0 || matched_idx < 0) return false;
    if ((ecs_mask[trigger_idx] & CMP_TRIGGER) == 0) return false;

    if ((cmp_trigger[trigger_idx].target_mask & CMP_PLAYER) != 0) {
        if ((ecs_mask[matched_idx] & CMP_PLAYER) == 0) return false;
        if (ecs_mask[trigger_idx] & CMP_GUN_CHARGER) {
            return player_has_grav_gun(handle_from_index(matched_idx)) && charger_available(trigger_idx);
        }
        return true;
    }

    return resource_held_for_storage(matched_idx);
}

static void sys_billboards_impl(float dt)
{
    (void)dt;
    for (int i=0;i<ECS_MAX_ENTITIES;++i){
        if (!ecs_alive_idx(i) || !(ecs_mask[i]&CMP_BILLBOARD)) continue;
        cmp_billboard[i].state = BILLBOARD_INACTIVE;
        cmp_billboard[i].timer = 0.0f;
    }

    ecs_prox_iter_t it = ecs_prox_stay_begin();
    ecs_prox_view_t v;
    while (ecs_prox_stay_next(&it, &v)){
        int trigger_idx = ent_index_checked(v.trigger_owner);
        int matched_idx = ent_index_checked(v.matched_entity);
        if (trigger_idx < 0 || matched_idx < 0) continue;
        if ((ecs_mask[trigger_idx] & CMP_BILLBOARD) == 0) continue;
        if ((ecs_mask[trigger_idx] & CMP_GRAV_GUN) && cmp_grav_gun[trigger_idx].held) continue;
        if (!billboard_should_activate(trigger_idx, matched_idx)) continue;

        cmp_billboard[trigger_idx].state = BILLBOARD_ACTIVE;
        float timer = cmp_billboard[trigger_idx].linger;
        if (timer <= 0.0f) timer = 1.0f;
        cmp_billboard[trigger_idx].timer = timer;
    }
}

// Public adapters (used by ecs_core.c)
SYSTEMS_ADAPT_VOID(sys_prox_build_adapt, sys_proximity_build_view_impl)
SYSTEMS_ADAPT_DT(sys_billboards_adapt, sys_billboards_impl)
