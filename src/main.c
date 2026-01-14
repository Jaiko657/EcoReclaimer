#include "engine/core/engine.h"
#include "game/ecs/ecs_game.h"

int main(void)
{
    engine_register_game_hooks((engine_game_hooks_t){
        .ecs_game_init = ecs_game_init,
        .ecs_game_shutdown = ecs_game_shutdown,
        .init_entities = init_entities,
        .ecs_find_player = ecs_find_player,
        .debug_str_register_game = debug_str_game_register_all,
        .render_game_ui = ecs_game_render_ui,
    });

    if (!engine_init("raylib + ECS: TARDAS MVP")) {
        return 1;
    }

    int code = engine_run();
    engine_shutdown();
    return code;
}
