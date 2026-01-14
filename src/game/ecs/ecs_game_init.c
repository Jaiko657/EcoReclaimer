//==== FROM ecs_game.c ====
#include "engine/ecs/ecs.h"
#include "game/ecs/ecs_game.h"
#include "engine/core/logger.h"
#include "engine/tiled/tiled.h"
#include "engine/prefab/pf_spawning.h"
#include "engine/world/world.h"
#include "engine/world/world_renderer.h"

bool init_entities(const char* tmx_path)
{
    const world_map_t* map = world_get_map();
    if (!map) {
        LOGC(LOGCAT_ECS, LOG_LVL_ERROR, "init_entities: no tiled map loaded");
        return false;
    }

    pf_spawn_from_map(map, tmx_path);
    return true;
}
