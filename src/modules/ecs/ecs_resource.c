#include "modules/ecs/ecs_resource.h"

#include "modules/ecs/ecs_internal.h"

#include <strings.h>

static const char* k_resource_type_names[] = {
    [RESOURCE_TYPE_PLASTIC] = "Plastic",
    [RESOURCE_TYPE_METAL] = "Metal",
};

static resource_type_t g_resource_type[ECS_MAX_ENTITIES];

bool resource_type_from_string(const char* s, resource_type_t* out_type)
{
    if (!out_type) return false;
    if (!s) {
        *out_type = RESOURCE_TYPE_PLASTIC;
        return true;
    }
    for (int i = 0; i < RESOURCE_TYPE_COUNT; ++i) {
        const char* name = k_resource_type_names[i];
        if (!name) continue;
        if (strcasecmp(name, s) == 0) {
            *out_type = (resource_type_t)i;
            return true;
        }
    }
    if (strcasecmp("RESOURCE", s) == 0) {
        *out_type = RESOURCE_TYPE_PLASTIC;
        return true;
    }
    return false;
}

const char* resource_type_to_string(resource_type_t type)
{
    if (type < 0 || type >= RESOURCE_TYPE_COUNT) return "Resource";
    return k_resource_type_names[type];
}

void cmp_add_resource(ecs_entity_t e, resource_type_t type)
{
    int idx = ent_index_checked(e);
    if (idx < 0) return;
    if (type < 0 || type >= RESOURCE_TYPE_COUNT) type = RESOURCE_TYPE_PLASTIC;
    g_resource_type[idx] = type;
    ecs_mask[idx] |= CMP_RESOURCE;
}

resource_type_t cmp_resource_type_from_index(int idx)
{
    if (idx < 0 || idx >= ECS_MAX_ENTITIES) return RESOURCE_TYPE_PLASTIC;
    if ((ecs_mask[idx] & CMP_RESOURCE) == 0) return RESOURCE_TYPE_PLASTIC;
    return g_resource_type[idx];
}

bool cmp_resource_get_type(ecs_entity_t e, resource_type_t* out_type)
{
    int idx = ent_index_checked(e);
    if (idx < 0) return false;
    if ((ecs_mask[idx] & CMP_RESOURCE) == 0) return false;
    if (out_type) *out_type = g_resource_type[idx];
    return true;
}

static void resource_destroy_hook(int idx)
{
    if (idx < 0 || idx >= ECS_MAX_ENTITIES) return;
    g_resource_type[idx] = RESOURCE_TYPE_PLASTIC;
}

void ecs_register_resource_component_hooks(void)
{
    ecs_register_component_destroy_hook(ENUM_RESOURCE, resource_destroy_hook);
}
