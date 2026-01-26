#include "game/prefab/pf_components_game.h"
#include "game/ecs/ecs_game.h"

static bool pf_component_gun_charger_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, void* out)
{
    (void)comp;
    (void)ovr;
    (void)out;
    return true;
}

static void pf_component_gun_charger_apply(ecs_entity_t e, const void* component)
{
    (void)component;
    cmp_add_gun_charger(e);
}

// Returns a pointer to the static ops struct for this component type.
const pf_component_ops_t* pf_component_gun_charger_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_GUN_CHARGER,
        .component_size = 0,
        .build = pf_component_gun_charger_build,
        .apply = pf_component_gun_charger_apply,
    };
    return &ops;
}
