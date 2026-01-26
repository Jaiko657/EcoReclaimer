#include "game/prefab/pf_components_game.h"
#include "game/ecs/ecs_game.h"

bool pf_component_recycle_bin_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_recycle_bin_t* out_recycle_bin)
{
    if (!out_recycle_bin) return false;
    resource_type_t type = RESOURCE_TYPE_PLASTIC;
    const char* value = pf_combined_value(comp, ovr, "recyclebintype");
    if (value) {
        resource_type_from_string(value, &type);
    }
    *out_recycle_bin = (pf_component_recycle_bin_t){ .type = type };
    return true;
}

static void pf_component_recycle_bin_apply(ecs_entity_t e, const void* component)
{
    const pf_component_recycle_bin_t* bin = (const pf_component_recycle_bin_t*)component;
    cmp_add_recycle_bin(e, bin->type);
}

// Returns a pointer to the static ops struct for this component type.
const pf_component_ops_t* pf_component_recycle_bin_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_RECYCLE_BIN,
        .component_size = sizeof(pf_component_recycle_bin_t),
        .build = (pf_build_fn)pf_component_recycle_bin_build,
        .apply = pf_component_recycle_bin_apply,
    };
    return &ops;
}
