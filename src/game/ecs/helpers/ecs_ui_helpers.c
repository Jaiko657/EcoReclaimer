#include "game/ecs/helpers/ecs_ui_helpers.h"

#include "game/ecs/helpers/ecs_player_helpers.h"
#include "game/ecs/ecs_game.h"

bool ecs_grav_gun_get_charge(float* out_charge, float* out_max)
{
    ecs_entity_t player = ecs_find_player();
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
