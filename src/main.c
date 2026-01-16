#include "engine/core/engine.h"
#include "game/ecs/ecs_game.h"

int main(void)
{
    //TODO: Replace this with Stage Based init (with registration of systems in central registry)
    // IE: PreEcs Registration, ECS Registration, PostECS Registration, Post Spawn, etc.
    engine_register_game_hooks((engine_game_hooks_t){
        .game_init = game_init,
        .ecs_game_shutdown = ecs_game_shutdown,
        .ecs_find_player = ecs_find_player,
        .render_game_ui = ecs_game_render_ui,
    });

    if (!engine_init("raylib + ECS: TARDAS MVP")) {
        return 1;
    }

    int code = engine_run();
    engine_shutdown();
    return code;
}
