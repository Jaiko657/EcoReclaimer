#include "game/game.h"

#include "game/ecs/ecs_game.h"
#include "game/prefab/pf_register_game.h"
#include "game/ui/ui.h"
#include "game/debug_str/debug_str_game.h"
#include "shared/utils/build_config.h"

// Init all game related subsystems, register all game related components, prefabs, ui layers, etc.
void game_init(void)
{
    pf_register_game_components();
    game_ui_register_layers();
#if DEBUG_BUILD
    debug_str_game_register_all();
#endif

    ecs_game_init();
}

void game_shutdown(void)
{
    ecs_game_shutdown();
}
