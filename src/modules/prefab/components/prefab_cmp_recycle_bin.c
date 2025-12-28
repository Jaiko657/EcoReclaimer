#include "modules/prefab/prefab_cmp.h"

bool prefab_cmp_recycle_bin_build(const prefab_component_t* comp, const tiled_object_t* obj, prefab_cmp_recycle_bin_t* out_recycle_bin)
{
    if (!out_recycle_bin) return false;

    const char* value = prefab_combined_value(comp, obj, "recyclebintype");
    resource_type_t type = RESOURCE_TYPE_PLASTIC;
    if (value && !resource_type_from_string(value, &type)) {
        type = RESOURCE_TYPE_PLASTIC;
    }

    *out_recycle_bin = (prefab_cmp_recycle_bin_t){ type };
    return true;
}
