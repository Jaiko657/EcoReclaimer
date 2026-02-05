#include "engine/engine/engine_manager/engine_manager.h"
#include "engine/core/logger/logger.h"
#include "engine/core/logger/logger_backend.h"
#include "engine/input/input.h"
#include "engine/asset/asset.h"
#include "engine/ecs/ecs.h"
#include "engine/ecs/ecs_engine.h"
#include "engine/ecs/ecs_physics.h"
#include "engine/runtime/toast.h"
#include "engine/renderer/renderer.h"
#include "engine/runtime/camera.h"
#include "engine/world/world_map.h"
#include "engine/engine/engine_scheduler/engine_scheduler.h"
#include "engine/engine/engine_scheduler/engine_register_systems.h"
#include "engine/core/platform/platform.h"
#include "engine/core/time/time.h"
#include "engine/engine/engine_phases/engine_phase.h"
#include "engine/prefab/registry/pf_registry.h"
#include "engine/prefab/loading/pf_loading.h"

#include <string.h>

static char g_current_tmx_path[256] = {0};

static void remember_tmx_path(const char* path)
{
    if (!path) return;
    strncpy(g_current_tmx_path, path, sizeof(g_current_tmx_path));
    g_current_tmx_path[sizeof(g_current_tmx_path) - 1] = '\0';
}

static bool engine_init_entities(const char* tmx_path)
{
    const world_map_t* map = world_get_map();
    if (!map) {
        LOGC(LOGCAT_ECS, LOG_LVL_ERROR, "init_entities: no tiled map loaded");
        return false;
    }
    pf_spawn_from_map(map, tmx_path);
    return true;
}

static bool reload_world_from_path(const char* tmx_path)
{
    if (!tmx_path) tmx_path = g_current_tmx_path;
    if (!tmx_path || tmx_path[0] == '\0') {
        LOGC(LOGCAT_MAIN, LOG_LVL_ERROR, "reload_world_from_path: no TMX path configured");
        return false;
    }

    char previous_path[sizeof(g_current_tmx_path)];
    strncpy(previous_path, g_current_tmx_path, sizeof(previous_path));
    previous_path[sizeof(previous_path) - 1] = '\0';

    if (!world_load_from_tmx(tmx_path, "walls")) {
        return false;
    }

    if (!renderer_bind_world_map()) {
        LOGC(LOGCAT_MAIN, LOG_LVL_ERROR, "Failed to load TMX map '%s' for renderer, reverting", tmx_path);
        if (strcmp(previous_path, tmx_path) != 0) {
            if (!world_load_from_tmx(previous_path, "walls")) {
                LOGC(LOGCAT_MAIN, LOG_LVL_FATAL, "Failed to revert world to previous TMX '%s'", previous_path);
            }
        }
        return false;
    }

    remember_tmx_path(tmx_path);
    return true;
}

bool engine_set_world_tmx_path(const char* tmx_path)
{
    if (!tmx_path || tmx_path[0] == '\0') {
        LOGC(LOGCAT_MAIN, LOG_LVL_ERROR, "engine_set_world_tmx_path: empty TMX path");
        return false;
    }

    if (!world_has_map()) {
        remember_tmx_path(tmx_path);
        return true;
    }

    return reload_world_from_path(tmx_path);
}

static bool engine_init_subsystems(const char *title)
{
    platform_init();
    logger_backend_init();
    log_set_min_level(LOG_LVL_DEBUG);

    ui_toast_init();
    engine_scheduler_init();
    input_init();
    asset_init();
    ecs_init();
    ecs_engine_init();
    camera_init();
    pf_register_engine_components(); // adds the handlers for engine components to prefab module
    renderer_ui_registry_init();
    engine_register_systems();
    engine_phase_run(ENGINE_PHASE_GAME_INIT);
    if (!g_current_tmx_path[0]) {
        LOGC(LOGCAT_MAIN, LOG_LVL_FATAL, "No startup TMX configured. Call engine_set_world_tmx_path() in game init.");
        return false;
    }
    if (!world_load_from_tmx(g_current_tmx_path, "walls")) {
        LOGC(LOGCAT_MAIN, LOG_LVL_FATAL, "Failed to load world collision");
        return false;
    }

    if (!renderer_init(1280, 720, title, 0)) {
        LOGC(LOGCAT_MAIN, LOG_LVL_FATAL, "renderer_init failed");
        return false;
    }

    if (!renderer_bind_world_map()) {
        LOGC(LOGCAT_MAIN, LOG_LVL_FATAL, "Failed to load TMX map");
        return false;
    }
    remember_tmx_path(g_current_tmx_path);

    // game entities/assets
    if (!engine_init_entities(g_current_tmx_path)) {
        LOGC(LOGCAT_MAIN, LOG_LVL_FATAL, "init_entities failed");
        return false;
    }
    engine_phase_run(ENGINE_PHASE_POST_ENTITIES);

    return true;
}

bool engine_init(const char *title)
{
    if (!engine_init_subsystems(title)) {
        engine_shutdown();
        return false;
    }
    return true;
}

int engine_run(void)
{
    const float FIXED_DT = 1.0f / 60.0f;
    float acc = 0.0f;

    while (!platform_should_close()) {
        platform_poll_events();
        input_begin_frame();

        float frame = time_frame_dt();
        if (frame > 0.25f) frame = 0.25f;  // avoid spiral of death
        acc += frame;

        while (acc >= FIXED_DT) {
            input_t in = input_for_tick();
            engine_scheduler_tick(FIXED_DT, &in);
            acc -= FIXED_DT;
        }
        engine_scheduler_present(frame);
    }

    return 0;
}

void engine_shutdown(void)
{
    ecs_phys_destroy_all();
    engine_phase_run(ENGINE_PHASE_PRE_SHUTDOWN);
    engine_phase_shutdown();
    ecs_engine_shutdown();
    ecs_shutdown();
    asset_shutdown();
    renderer_shutdown();
    camera_shutdown();
    world_shutdown();
}

bool engine_reload_world(void)
{
    // Keep current camera position when hot-reloading the same TMX; bounds will clamp.
    return reload_world_from_path(g_current_tmx_path);
}

bool engine_reload_world_from_path(const char* tmx_path)
{
    // When reloading a specific TMX (e.g. hot reload), avoid snapping to spawn.
    return reload_world_from_path(tmx_path);
}
