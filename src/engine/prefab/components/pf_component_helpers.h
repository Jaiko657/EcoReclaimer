#pragma once

// Shared prefab parsing helpers used by engine and game components.

#include <stdbool.h>

#include "engine/ecs/ecs.h"
#include "engine/prefab/prefab.h"
#include "engine/tiled/tiled.h"

typedef struct pf_override_ctx_t {
    const tiled_object_t* obj;
    bool enabled;
} pf_override_ctx_t;

static inline const tiled_object_t* pf_override_obj(const pf_override_ctx_t* ovr)
{
    return (ovr && ovr->enabled) ? ovr->obj : NULL;
}

ComponentMask pf_parse_mask(const char* s, bool* out_ok);
bool     pf_parse_float(const char* s, float* out_v);
bool     pf_parse_int(const char* s, int* out_v);
facing_t pf_parse_facing(const char* s, facing_t fallback);

const char* pf_find_prop(const prefab_component_t* comp, const char* field);
const char* pf_override_value(const prefab_component_t* comp, const pf_override_ctx_t* ovr, const char* field);
const char* pf_combined_value(const prefab_component_t* comp, const pf_override_ctx_t* ovr, const char* field);

v2f pf_object_position_default(const tiled_object_t* obj);
const char* pf_object_prop_only(const pf_override_ctx_t* ovr, const char* comp_name, const char* field);
