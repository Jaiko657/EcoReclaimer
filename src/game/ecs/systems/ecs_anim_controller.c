//==== FROM ecs_anim_controller.c ====
#include "game/ecs/ecs_game.h"
#include "game/ecs/helpers/ecs_player_helpers.h"
#include "engine/core/logger/logger.h"
#include "engine/engine/engine_scheduler/engine_scheduler.h"

static void sys_anim_controller_impl(void)
{
    ecs_entity_t player = ecs_find_player();
    int idx = ent_index_checked(player);
    if (idx < 0) return;

    if ((ecs_mask[idx] & (CMP_ANIM | CMP_VEL)) != (CMP_ANIM | CMP_VEL))
        return;

    cmp_anim_t*      a = &cmp_anim[idx];
    cmp_velocity_t*  v = &cmp_vel[idx];

    float vx = v->x;
    float vy = v->y;
    float speed2 = vx*vx + vy*vy;

    int dir = (int)v->facing.facingDir;   // 0..7

    int new_anim;
    if (speed2 < 0.01f * 0.01f) {
        // Idle
        new_anim = 8 + dir;
    } else {
        // Walking
        new_anim = dir;
    }

    if(new_anim >= MAX_ANIMS) {
        LOGC(LOGCAT_ECS, LOG_LVL_ERROR, "New animation: %i outside of max animation %i", new_anim, MAX_ANIMS);
        return;
    }
    if (new_anim != a->current_anim) {
        a->current_anim = new_anim;
        a->frame_index  = 0;
        a->current_time = 0.0f;
    }

    if (!a->frames_per_anim || !a->anim_offsets || !a->frames) return;

    int seq_len = a->frames_per_anim[a->current_anim];
    if (seq_len <= 0) return;
    if (a->frame_index >= seq_len) {
        a->frame_index = 0;
    }
}

SYSTEMS_ADAPT_VOID(sys_anim_controller_adapt, sys_anim_controller_impl)
