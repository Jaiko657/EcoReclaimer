#include "game/prefab/pf_components_game.h"
#include "game/ecs/ecs_storage.h"

bool pf_component_storage_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_storage_t* out_storage)
{
    if (!out_storage) return false;
    *out_storage = (pf_component_storage_t){0};
    const char* value = pf_combined_value(comp, ovr, "capacity");
    if (pf_parse_int(value, &out_storage->capacity)) {
        out_storage->has_capacity = true;
    }
    return true;
}

static void pf_component_storage_apply(ecs_entity_t e, const void* component)
{
    const pf_component_storage_t* storage = (const pf_component_storage_t*)component;
    cmp_add_storage(e, storage->has_capacity ? storage->capacity : 0);
}

const pf_component_ops_t* pf_component_storage_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_STORAGE,
        .component_size = sizeof(pf_component_storage_t),
        .build = (pf_build_fn)pf_component_storage_build,
        .apply = pf_component_storage_apply,
    };
    return &ops;
}
