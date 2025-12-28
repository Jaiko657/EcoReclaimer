#include "modules/prefab/prefab_cmp.h"

bool prefab_cmp_storage_build(const prefab_component_t* comp, const tiled_object_t* obj, prefab_cmp_storage_t* out_storage)
{
    if (!out_storage) return false;
    *out_storage = (prefab_cmp_storage_t){0};

    const char* value = prefab_combined_value(comp, obj, "capacity");
    int capacity = 0;
    if (prefab_parse_int(value, &capacity)) {
        out_storage->capacity = capacity;
        out_storage->has_capacity = true;
    }

    return true;
}
