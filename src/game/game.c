#include "game/game.h"

#include "game/ecs/ecs_game.h"
#include "game/prefab/pf_register_game.h"
#include "game/ui/ui.h"
#include "game/debug_str/debug_str_game.h"
#include "engine/engine/engine_manager/engine_manager.h"
#include "game/ecs/game_register_systems.h"

// Init all game related subsystems, register all game related components, prefabs, ui layers, etc.
void game_init(void)
{
    engine_set_world_tmx_path("assets/maps/start.tmx");
    pf_register_game_components();
    game_ui_register_layers();
#if DEBUG_BUILD
    debug_str_game_register_all();
#endif

    ecs_game_init();
    game_register_systems();
}

void game_shutdown(void)
{
    ecs_game_shutdown();
}
