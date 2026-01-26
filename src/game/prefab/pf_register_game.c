#include "game/prefab/pf_register_game.h"

#include "engine/prefab/registry/pf_registry.h"
#include "game/prefab/pf_components_game.h"

// Register all game related prefabs parsing ops for each game component
void pf_register_game_components(void)
{
    pf_register_set(pf_component_player_ops());
    pf_register_set(pf_component_resource_ops());
    pf_register_set(pf_component_storage_ops());
    pf_register_set(pf_component_recycle_bin_ops());
    pf_register_set(pf_component_liftable_ops());
    pf_register_set(pf_component_conveyor_ops());
    pf_register_set(pf_component_door_ops());
    pf_register_set(pf_component_grav_gun_ops());
    pf_register_set(pf_component_gun_charger_ops());
    pf_register_set(pf_component_unpacker_ops());
    pf_register_set(pf_component_unloader_ops());
}
