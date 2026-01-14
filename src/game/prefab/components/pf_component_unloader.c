#include "game/prefab/pf_components_game.h"
#include "engine/ecs/ecs.h"

bool pf_component_unloader_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_unloader_t* out_unloader)
{
    (void)comp;
    (void)ovr;
    if (!out_unloader) return false;
    *out_unloader = (pf_component_unloader_t){0};
    return true;
}

static void pf_component_unloader_apply(ecs_entity_t e, const void* component)
{
    (void)component;
    cmp_add_unloader(e, ecs_null());
}

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
