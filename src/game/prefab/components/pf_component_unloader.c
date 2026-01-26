#include "game/prefab/pf_components_game.h"
#include "game/ecs/ecs_game.h"

bool pf_component_unloader_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_unloader_t* out_unloader)
{
    (void)comp;
    (void)ovr;
    if (!out_unloader) return false;
    *out_unloader = (pf_component_unloader_t){ .unused = 0 };
    return true;
}

static void pf_component_unloader_apply(ecs_entity_t e, const void* component)
{
    (void)component;
    cmp_add_unloader(e, ecs_null());
}

// Returns a pointer to the static ops struct for this component type.
const pf_component_ops_t* pf_component_unloader_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_UNLOADER,
        .component_size = sizeof(pf_component_unloader_t),
        .build = (pf_build_fn)pf_component_unloader_build,
        .apply = pf_component_unloader_apply,
    };
    return &ops;
}
