#include "modules/prefab/prefab_cmp.h"

bool prefab_cmp_resource_build(const prefab_component_t* comp, const tiled_object_t* obj, prefab_cmp_resource_t* out_resource)
{
    if (!out_resource) return false;
    const char* value = prefab_combined_value(comp, obj, "resource_type");
    resource_type_t type = RESOURCE_TYPE_PLASTIC;
    if (value && !resource_type_from_string(value, &type)) {
        type = RESOURCE_TYPE_PLASTIC;
    }
    *out_resource = (prefab_cmp_resource_t){ type };
    return true;
}
