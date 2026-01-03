#include "modules/prefab/prefab_cmp.h"

bool prefab_cmp_unloader_build(const prefab_component_t* comp, const tiled_object_t* obj, prefab_cmp_unloader_t* out_unloader)
{
    (void)comp;
    (void)obj;
    if (!out_unloader) return false;
    *out_unloader = (prefab_cmp_unloader_t){0};
    return true;
}
