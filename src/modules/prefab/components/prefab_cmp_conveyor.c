#include "modules/prefab/prefab_cmp.h"

bool prefab_cmp_conveyor_build(const prefab_component_t* comp, const tiled_object_t* obj, prefab_cmp_conveyor_t* out_conveyor)
{
    if (!out_conveyor) return false;
    facing_t direction = DIR_EAST;
    const char* dir_key = prefab_combined_value(comp, obj, "dir");
    if (!dir_key) dir_key = prefab_combined_value(comp, obj, "direction");
    direction = prefab_parse_facing(dir_key, direction);

    float speed = 0.0f;
    prefab_parse_float(prefab_combined_value(comp, obj, "speed"), &speed);

    int block_player_input = 1;
    const char* block = prefab_combined_value(comp, obj, "block_player_input");
    if (!block) block = prefab_combined_value(comp, obj, "disable_player_input");
    if (block) prefab_parse_int(block, &block_player_input);

    *out_conveyor = (prefab_cmp_conveyor_t){
        .direction = direction,
        .speed = speed,
        .block_player_input = block_player_input != 0
    };
    return true;
}
