#include "game/ecs/ecs_game.h"
#include "game/ecs/ecs_billboards_filter.h"
#include "game/ecs/ecs_game_internal.h"
#include "game/ecs/helpers/ecs_player_helpers.h"
#include "game/ecs/helpers/ecs_resource_helpers.h"
#include "engine/engine/engine_phases/engine_phase.h"
#include "engine/runtime/camera.h"
#include "engine/core/logger/logger.h"

// =============== ECS Storage =============
cmp_player_t    cmp_player[ECS_MAX_ENTITIES];
cmp_conveyor_t  cmp_conveyor[ECS_MAX_ENTITIES];
cmp_conveyor_rider_t cmp_conveyor_rider[ECS_MAX_ENTITIES];
cmp_liftable_t  cmp_liftable[ECS_MAX_ENTITIES];
cmp_grav_gun_t  cmp_grav_gun[ECS_MAX_ENTITIES];
cmp_gun_charger_t cmp_gun_charger[ECS_MAX_ENTITIES];
cmp_door_t      cmp_door[ECS_MAX_ENTITIES];
cmp_unloader_t  cmp_unloader[ECS_MAX_ENTITIES];
cmp_unpacker_t  cmp_unpacker[ECS_MAX_ENTITIES];
resource_type_t cmp_resource_type[ECS_MAX_ENTITIES];
cmp_storage_t   cmp_storage[ECS_MAX_ENTITIES];

// Defered function ran after entities are created in engine so camera can lock to player
static void game_post_entities(engine_phase_t phase, void* data)
{
    (void)phase;
    (void)data;
    camera_set_target(ecs_find_player());
}

// inits the ecs game related systems, components and hooks
void ecs_game_init(void)
{
    ecs_register_grav_gun_component_hooks();
    ecs_register_liftable_component_hooks();
    ecs_register_resource_component_hooks();
    ecs_register_component_destroy_hook(ENUM_DOOR, ecs_door_on_destroy);
    ecs_billboards_register_game_filter();
    engine_phase_register(ENGINE_PHASE_POST_ENTITIES, 0, game_post_entities, NULL, "game_post_entities");
}

//Might need this no idea yet
void ecs_game_shutdown(void)
{
}
