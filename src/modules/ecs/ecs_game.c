#include "modules/ecs/ecs.h"
#include "modules/ecs/ecs_game.h"
#include "modules/core/logger.h"
#include "modules/tiled/tiled.h"
#include "modules/ecs/ecs_prefab_loading.h"
#include "modules/world/world.h"
#include "modules/world/world_renderer.h"

bool init_entities(const char* tmx_path)
{
    const world_map_t* map = world_get_map();
    if (!map) {
        LOGC(LOGCAT_ECS, LOG_LVL_ERROR, "init_entities: no tiled map loaded");
        return false;
    }

    ecs_prefab_spawn_from_map(map, tmx_path);
    return true;
}
