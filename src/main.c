#include "engine/engine/engine_manager/engine_manager.h"
#include "engine/engine/engine_phases/engine_phase.h"
#include "game/game.h"

static void game_init_phase(engine_phase_t phase, void* data)
{
    (void)phase;
    (void)data;
    game_init();
}

static void game_shutdown_phase(engine_phase_t phase, void* data)
{
    (void)phase;
    (void)data;
    game_shutdown();
}

int main(void)
{
    engine_phase_init();
    engine_phase_register(ENGINE_PHASE_GAME_INIT, 0, game_init_phase, NULL, "game_init");
    engine_phase_register(ENGINE_PHASE_PRE_SHUTDOWN, 0, game_shutdown_phase, NULL, "game_shutdown");

    if (!engine_init("Eco Reclaimer: 0.3")) {
        return 1;
    }

    int code = engine_run();
    engine_shutdown();
    return code;
}
