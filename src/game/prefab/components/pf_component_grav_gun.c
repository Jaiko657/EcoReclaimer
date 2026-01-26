#include "game/prefab/pf_components_game.h"
#include "game/ecs/ecs_game.h"

static void pf_component_grav_gun_apply(ecs_entity_t e, const void* component)
{
    (void)component;
    cmp_add_grav_gun(e);
}

// Returns a pointer to the static ops struct for this component type.
const pf_component_ops_t* pf_component_grav_gun_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_GRAV_GUN,
        .component_size = sizeof(pf_component_grav_gun_t),
        .build = (pf_build_fn)pf_component_grav_gun_build,
        .apply = pf_component_grav_gun_apply,
    };
    return &ops;
}
