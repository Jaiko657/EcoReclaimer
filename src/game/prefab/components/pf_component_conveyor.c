#include "game/prefab/pf_components_game.h"
#include "engine/ecs/ecs.h"

bool pf_component_conveyor_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_conveyor_t* out_conveyor)
{
    if (!out_conveyor) return false;
    facing_t direction = DIR_EAST;
    const char* dir_key = pf_combined_value(comp, ovr, "dir");
    if (!dir_key) dir_key = pf_combined_value(comp, ovr, "direction");
    direction = pf_parse_facing(dir_key, direction);

    float speed = 0.0f;
    pf_parse_float(pf_combined_value(comp, ovr, "speed"), &speed);

    int block_player_input = 1;
    const char* block = pf_combined_value(comp, ovr, "block_player_input");
    if (!block) block = pf_combined_value(comp, ovr, "disable_player_input");
    if (block) pf_parse_int(block, &block_player_input);

    *out_conveyor = (pf_component_conveyor_t){
        .direction = direction,
        .speed = speed,
        .block_player_input = block_player_input != 0
    };
    return true;
}

static void pf_component_conveyor_apply(ecs_entity_t e, const void* component)
{
    const pf_component_conveyor_t* conveyor = (const pf_component_conveyor_t*)component;
    cmp_add_conveyor(e, conveyor->direction, conveyor->speed, conveyor->block_player_input);
}

const pf_component_ops_t* pf_component_conveyor_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_CONVEYOR,
        .component_size = sizeof(pf_component_conveyor_t),
        .build = (pf_build_fn)pf_component_conveyor_build,
        .apply = pf_component_conveyor_apply,
    };
    return &ops;
}
