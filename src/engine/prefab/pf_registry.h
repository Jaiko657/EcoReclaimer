#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "engine/ecs/ecs.h"
#include "engine/prefab/components/pf_component_helpers.h"
#include "engine/prefab/prefab.h"

typedef bool (*pf_build_fn)(const prefab_component_t* comp, const pf_override_ctx_t* ovr, void* out_component);
typedef void (*pf_apply_fn)(ecs_entity_t e, const void* component);
typedef void (*pf_free_fn)(void* component);
typedef void (*pf_override_fn)(const pf_override_ctx_t* ovr, void* component);

typedef struct pf_component_ops_t {
    ComponentEnum id;
    size_t component_size;
    pf_build_fn build;
    pf_apply_fn apply;
    pf_free_fn free;
    pf_override_fn override;
    bool override_if_missing;
} pf_component_ops_t;

bool pf_register_set(const pf_component_ops_t* ops);
const pf_component_ops_t* pf_register_get(ComponentEnum id);

// Registers all engine-owned prefab component handlers.
void pf_register_engine_components(void);
