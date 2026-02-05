//==== FROM ecs_conveyor.c ====
#include "game/ecs/ecs_game.h"
#include "engine/ecs/ecs_proximity.h"
#include "engine/engine/engine_scheduler/engine_scheduler.h"

#include <math.h>

static void facing_to_vector(facing_t dir, float* out_x, float* out_y)
{
    static const float k_diag = 0.7071067811865475f;
    static const float k_dx[] = {
        0.0f,   k_diag, 1.0f,  k_diag,  0.0f,  -k_diag, -1.0f, -k_diag
    };
    static const float k_dy[] = {
        -1.0f, -k_diag, 0.0f,  k_diag,  1.0f,   k_diag,  0.0f, -k_diag
    };

    int idx = (int)dir;
    if (idx < 0 || idx >= (int)(sizeof(k_dx) / sizeof(k_dx[0]))) {
        idx = DIR_EAST;
    }
    if (out_x) *out_x = k_dx[idx];
    if (out_y) *out_y = k_dy[idx];
}

static bool conveyor_rider_is_held(int idx)
{
    return (ecs_mask[idx] & CMP_LIFTABLE) && (cmp_liftable[idx].state == GRAV_GUN_STATE_HELD);
}

static void conveyor_enter_rider(int idx, cmp_conveyor_rider_t* rider)
{
    if (!rider || (ecs_mask[idx] & CMP_PHYS_BODY) == 0) return;
    if (rider->active_count == 0) {
        cmp_phys_body[idx].type = PHYS_KINEMATIC;
    }
    rider->active_count += 1;
}

static void conveyor_exit_rider(int idx, cmp_conveyor_rider_t* rider)
{
    if (!rider || (ecs_mask[idx] & CMP_PHYS_BODY) == 0) return;
    if (rider->active_count > 0) {
        rider->active_count -= 1;
    }
    if (rider->active_count == 0) {
        cmp_phys_body[idx].type = cmp_phys_body[idx].default_type;
        cmp_phys_body[idx].category_bits = cmp_phys_body[idx].default_category_bits;
        cmp_phys_body[idx].mask_bits = cmp_phys_body[idx].default_mask_bits;
        *rider = (cmp_conveyor_rider_t){ .active_count = 0 };
        ecs_mask[idx] &= ~CMP_CONVEYOR_RIDER;
    }
}

static void conveyor_force_exit(int idx, cmp_conveyor_rider_t* rider)
{
    if (!rider || (ecs_mask[idx] & CMP_PHYS_BODY) == 0) return;
    rider->active_count = 0;
    cmp_phys_body[idx].type = cmp_phys_body[idx].default_type;
    cmp_phys_body[idx].category_bits = cmp_phys_body[idx].default_category_bits;
    cmp_phys_body[idx].mask_bits = cmp_phys_body[idx].default_mask_bits;
    *rider = (cmp_conveyor_rider_t){ .active_count = 0 };
    ecs_mask[idx] &= ~CMP_CONVEYOR_RIDER;
}

static void sys_conveyor_update_impl(void)
{
    float belt_vel_x[ECS_MAX_ENTITIES] = {0};
    float belt_vel_y[ECS_MAX_ENTITIES] = {0};
    bool belt_block_input[ECS_MAX_ENTITIES] = {0};

    ecs_prox_iter_t enter_it = ecs_prox_enter_begin();
    ecs_prox_view_t v;
    while (ecs_prox_enter_next(&enter_it, &v)) {
        int belt_idx = ent_index_checked(v.trigger_owner);
        int rider_idx = ent_index_checked(v.matched_entity);
        if (belt_idx < 0 || rider_idx < 0) continue;
        if ((ecs_mask[belt_idx] & CMP_CONVEYOR) == 0) continue;
        if ((ecs_mask[rider_idx] & CMP_PHYS_BODY) == 0) continue;
        if (conveyor_rider_is_held(rider_idx)) continue;

        if ((ecs_mask[rider_idx] & CMP_CONVEYOR_RIDER) == 0) {
            cmp_conveyor_rider[rider_idx] = (cmp_conveyor_rider_t){ .active_count = 0 };
            ecs_mask[rider_idx] |= CMP_CONVEYOR_RIDER;
        }
        conveyor_enter_rider(rider_idx, &cmp_conveyor_rider[rider_idx]);
    }

    ecs_prox_iter_t exit_it = ecs_prox_exit_begin();
    while (ecs_prox_exit_next(&exit_it, &v)) {
        int belt_idx = ent_index_checked(v.trigger_owner);
        int rider_idx = ent_index_checked(v.matched_entity);
        if (belt_idx < 0 || rider_idx < 0) continue;
        if ((ecs_mask[belt_idx] & CMP_CONVEYOR) == 0) continue;
        if ((ecs_mask[rider_idx] & CMP_CONVEYOR_RIDER) == 0) continue;
        conveyor_exit_rider(rider_idx, &cmp_conveyor_rider[rider_idx]);
    }

    ecs_prox_iter_t it = ecs_prox_stay_begin();
    while (ecs_prox_stay_next(&it, &v)) {
        int belt_idx = ent_index_checked(v.trigger_owner);
        int rider_idx = ent_index_checked(v.matched_entity);
        if (belt_idx < 0 || rider_idx < 0) continue;
        if ((ecs_mask[belt_idx] & CMP_CONVEYOR) == 0) continue;
        if ((ecs_mask[rider_idx] & CMP_PHYS_BODY) == 0) continue;
        if (conveyor_rider_is_held(rider_idx)) continue;

        if ((ecs_mask[rider_idx] & CMP_CONVEYOR_RIDER) == 0) {
            cmp_conveyor_rider[rider_idx] = (cmp_conveyor_rider_t){ .active_count = 0 };
            ecs_mask[rider_idx] |= CMP_CONVEYOR_RIDER;
            conveyor_enter_rider(rider_idx, &cmp_conveyor_rider[rider_idx]);
        }

        const cmp_conveyor_t* belt = &cmp_conveyor[belt_idx];
        float dir_x = 0.0f;
        float dir_y = 0.0f;
        facing_to_vector(belt->direction, &dir_x, &dir_y);
        float cand_vx = dir_x * belt->speed;
        float cand_vy = dir_y * belt->speed;
        float cand_speed = sqrtf(cand_vx * cand_vx + cand_vy * cand_vy);
        float cur_speed = sqrtf(belt_vel_x[rider_idx] * belt_vel_x[rider_idx] +
                                belt_vel_y[rider_idx] * belt_vel_y[rider_idx]);
        if (cand_speed > cur_speed) {
            belt_vel_x[rider_idx] = cand_vx;
            belt_vel_y[rider_idx] = cand_vy;
        }
        if (belt->block_player_input) {
            belt_block_input[rider_idx] = true;
        }
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_CONVEYOR_RIDER) == 0) continue;

        cmp_conveyor_rider_t* rider = &cmp_conveyor_rider[i];
        if (conveyor_rider_is_held(i)) {
            conveyor_force_exit(i, rider);
            continue;
        }

        rider->vel_x = belt_vel_x[i];
        rider->vel_y = belt_vel_y[i];
        rider->block_player_input = belt_block_input[i];
    }
}

static void sys_conveyor_apply_impl(void)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & (CMP_CONVEYOR_RIDER | CMP_VEL | CMP_PHYS_BODY)) !=
            (CMP_CONVEYOR_RIDER | CMP_VEL | CMP_PHYS_BODY)) {
            continue;
        }
        if (cmp_conveyor_rider[i].active_count <= 0) continue;
        cmp_vel[i].x = cmp_conveyor_rider[i].vel_x;
        cmp_vel[i].y = cmp_conveyor_rider[i].vel_y;
    }
}

SYSTEMS_ADAPT_VOID(sys_conveyor_update_adapt, sys_conveyor_update_impl)
SYSTEMS_ADAPT_VOID(sys_conveyor_apply_adapt, sys_conveyor_apply_impl)
