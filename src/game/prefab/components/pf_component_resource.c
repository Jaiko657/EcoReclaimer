#include "game/prefab/pf_components_game.h"
#include "game/ecs/ecs_game.h"

bool pf_component_resource_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_resource_t* out_resource)
{
    if (!out_resource) return false;
    resource_type_t type = RESOURCE_TYPE_PLASTIC;
    const char* value = pf_combined_value(comp, ovr, "resource_type");
    if (!value) value = pf_combined_value(comp, ovr, "resource");
    if (value) {
        resource_type_from_string(value, &type);
    }
    *out_resource = (pf_component_resource_t){ .type = type };
    return true;
}

static void pf_component_resource_apply(ecs_entity_t e, const void* component)
{
    const pf_component_resource_t* res = (const pf_component_resource_t*)component;
    cmp_add_resource(e, res->type);
}

// Returns a pointer to the static ops struct for this component type.
const pf_component_ops_t* pf_component_resource_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_RESOURCE,
        .component_size = sizeof(pf_component_resource_t),
        .build = (pf_build_fn)pf_component_resource_build,
        .apply = pf_component_resource_apply,
    };
    return &ops;
}
