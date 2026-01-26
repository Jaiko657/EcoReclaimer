#include "game/ecs/ecs_billboards_filter.h"
#include "game/ecs/ecs_game.h"
#include "engine/ecs/ecs_billboards.h"

static bool resource_held_for_storage(int idx)
{
    // True only for liftable resources currently held by the grav gun (eligible for storage).
    if ((ecs_mask[idx] & (CMP_RESOURCE | CMP_LIFTABLE)) != (CMP_RESOURCE | CMP_LIFTABLE)) return false;
    return cmp_liftable[idx].state == GRAV_GUN_STATE_HELD;
}

static bool player_has_grav_gun(ecs_entity_t player)
{
    // Validate the player entity and its held gun pointer; clear stale state if invalid.
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

static bool billboard_filter_requires_trigger(int trigger_idx, int matched_idx, void* data)
{
    // Filter: only allow billboards for entities that have a trigger component.
    (void)data;
    (void)matched_idx;
    if (trigger_idx < 0) return false;
    if ((ecs_mask[trigger_idx] & CMP_TRIGGER) == 0) return false;
    return true;
}

static bool billboard_filter_ignore_held_gun(int trigger_idx, int matched_idx, void* data)
{
    // Filter: suppress billboards for a grav gun entity while it is held.
    (void)matched_idx;
    (void)data;
    if (trigger_idx < 0) return false;
    if ((ecs_mask[trigger_idx] & CMP_GRAV_GUN) && cmp_grav_gun[trigger_idx].held) return false;
    return true;
}

static bool billboard_filter_player_targeting(int trigger_idx, int matched_idx, void* data)
{
    // Filter: when a trigger targets players, the matched entity must be a player.
    // Special-case gun chargers: show only if the player holds a gun and the charger is empty.
    (void)data;
    if (trigger_idx < 0 || matched_idx < 0) return false;
    if ((cmp_trigger[trigger_idx].target_mask & CMP_PLAYER) == 0) return true;
    if ((ecs_mask[matched_idx] & CMP_PLAYER) == 0) return false;
    if (ecs_mask[trigger_idx] & CMP_GUN_CHARGER) {
        ecs_entity_t stored = cmp_gun_charger[trigger_idx].stored_gun;
        const bool charger_empty = !ecs_alive_handle(stored);
        return player_has_grav_gun(handle_from_index(matched_idx)) && charger_empty;
    }
    return true;
}

static bool billboard_filter_resource_targeting(int trigger_idx, int matched_idx, void* data)
{
    // Filter: for non-player triggers, only allow held resources intended for storage.
    (void)data;
    if (trigger_idx < 0 || matched_idx < 0) return false;
    if ((cmp_trigger[trigger_idx].target_mask & CMP_PLAYER) != 0) return true;
    return resource_held_for_storage(matched_idx);
}

void ecs_billboards_register_game_filter(void)
{
    // Register the game-specific billboard filters in evaluation order.
    ecs_billboards_clear_filters();
    ecs_billboards_add_filter(billboard_filter_requires_trigger, NULL);
    ecs_billboards_add_filter(billboard_filter_ignore_held_gun, NULL);
    ecs_billboards_add_filter(billboard_filter_player_targeting, NULL);
    ecs_billboards_add_filter(billboard_filter_resource_targeting, NULL);
}
