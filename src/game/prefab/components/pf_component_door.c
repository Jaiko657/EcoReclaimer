#include "game/prefab/pf_components_game.h"
#include "engine/core/logger/logger.h"
#include "game/ecs/ecs_game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool parse_door_tiles(const char* s, pf_door_tile_xy_list_t* out_xy, int* out_count)
{
    if (out_count) *out_count = 0;
    if (!out_xy || !s || !out_count) {
        LOGC(LOGCAT_PREFAB, LOG_LVL_WARN, "parse_door_tiles: invalid args");
        return false;
    }
    int count = 0;
    const char* p = s;
    while (*p) {
        int x = 0, y = 0;
        if (sscanf(p, "%d,%d", &x, &y) == 2) {
            door_tile_xy_t tile = { x, y };
            DA_APPEND(out_xy, tile);
            count++;
        }
        const char* sep = strchr(p, ';');
        if (!sep) break;
        p = sep + 1;
    }
    *out_count = count;
    return count > 0;
}

bool pf_component_door_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_door_t* out_door)
{
    if (!out_door) return false;
    const tiled_object_t* obj = pf_override_obj(ovr);
    float prox_r = 0.0f;
    pf_parse_float(pf_combined_value(comp, ovr, "proximity_radius"), &prox_r);

    *out_door = (pf_component_door_t){ .prox_radius = 0.0f };
    out_door->prox_radius = prox_r;

    int tile_count = 0;
    parse_door_tiles(pf_combined_value(comp, ovr, "door_tiles"), &out_door->tiles, &tile_count);
    if (tile_count <= 0 && obj && obj->door_tile_count > 0) {
        for (int i = 0; i < obj->door_tile_count; ++i) {
            door_tile_xy_t tile = { obj->door_tiles[i][0], obj->door_tiles[i][1] };
            DA_APPEND(&out_door->tiles, tile);
        }
        tile_count = obj->door_tile_count;
    }
    if (tile_count <= 0) {
        const char* obj_name = obj && obj->name ? obj->name : "(null)";
        const char* layer_name = obj && obj->layer_name ? obj->layer_name : "(null)";
        int obj_id = obj ? obj->id : -1;
        LOGC(LOGCAT_PREFAB, LOG_LVL_ERROR, "prefab door missing tiles (obj=%s layer=%s id=%d): set DOOR.door_tiles or obj door tiles", obj_name, layer_name, obj_id);
    }
    return tile_count > 0;
}

void pf_component_door_free(pf_component_door_t* door)
{
    if (!door) return;
    DA_FREE(&door->tiles);
    *door = (pf_component_door_t){ .prox_radius = 0.0f };
}

static void pf_component_door_apply(ecs_entity_t e, const void* component)
{
    const pf_component_door_t* door = (const pf_component_door_t*)component;
    cmp_add_door(e, door->prox_radius, (int)door->tiles.size,
                door->tiles.size ? door->tiles.data : NULL);
}

static void pf_component_door_free_component(void* component)
{
    pf_component_door_free((pf_component_door_t*)component);
}

// Returns a pointer to the static ops struct for this component type.
const pf_component_ops_t* pf_component_door_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_DOOR,
        .component_size = sizeof(pf_component_door_t),
        .build = (pf_build_fn)pf_component_door_build,
        .apply = pf_component_door_apply,
        .free = pf_component_door_free_component,
    };
    return &ops;
}
