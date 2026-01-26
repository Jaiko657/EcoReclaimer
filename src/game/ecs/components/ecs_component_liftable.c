#include "game/ecs/ecs_game.h"

void cmp_add_liftable(ecs_entity_t e)
{
    int i = ent_index_checked(e);
    if (i < 0) return;
    cmp_liftable[i] = (cmp_liftable_t){
        .state              = GRAV_GUN_STATE_FREE,
        .holder             = ecs_null(),
        .pickup_distance    = 0.0f,
        .pickup_radius      = 0.0f,
        .max_hold_distance  = 0.0f,
        .breakoff_distance  = 0.0f,
        .follow_gain        = 0.0f,
        .max_speed          = 0.0f,
        .damping            = 0.0f,
        .hold_vel_x         = 0.0f,
        .hold_vel_y         = 0.0f,
        .grab_offset_x      = 0.0f,
        .grab_offset_y      = 0.0f,
        .just_dropped       = false,
        .recycle_active     = false,
        .recycle_target_y   = 0.0f
    };
    ecs_mask[i] |= CMP_LIFTABLE;
}
