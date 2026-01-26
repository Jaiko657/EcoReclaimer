#include "engine/prefab/loading/pf_loading.h"

#include "engine/core/logger/logger.h"
#include "engine/ecs/ecs_core.h"
#include "engine/prefab/components/pf_component_helpers.h"
#include "engine/prefab/registry/pf_registry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char* xstrdup_local(const char* s)
{
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char* p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

typedef struct pf_loading_built_entity_t {
    ComponentMask present_mask;
    ComponentMask built_mask;
    ComponentMask override_mask;
    void* component_data[ENUM_COMPONENT_COUNT];
} pf_loading_built_entity_t;

static void pf_loading_built_entity_free(pf_loading_built_entity_t* built)
{
    if (!built) return;
    for (int i = 0; i < ENUM_COMPONENT_COUNT; ++i) {
        if (!built->component_data[i]) continue;
        const pf_component_ops_t* ops = pf_register_get((ComponentEnum)i);
        if (ops && ops->free) {
            ops->free(built->component_data[i]);
        }
        free(built->component_data[i]);
    }
    *built = (pf_loading_built_entity_t){ .present_mask = 0 };
}

static void pf_loading_build_entity_components(pf_loading_built_entity_t* built, const prefab_t* prefab, const pf_override_ctx_t* ovr)
{
    if (!built || !prefab) return;

    for (size_t i = 0; i < prefab->component_count; ++i) {
        const prefab_component_t* comp = &prefab->components[i];
        built->present_mask |= (1ull << comp->id);
        if (comp->override_after_spawn) built->override_mask |= (1ull << comp->id);

        const pf_component_ops_t* ops = pf_register_get(comp->id);
        if (!ops) {
            LOGC(LOGCAT_PREFAB, LOG_LVL_FATAL, "prefab: missing handler for component %d", (int)comp->id);
            abort();
        }

        if (ops->component_size > 0) {
            void* data = calloc(1, ops->component_size);
            if (!data) {
                LOGC(LOGCAT_PREFAB, LOG_LVL_FATAL, "prefab: allocation failed for component %d", (int)comp->id);
                abort();
            }
            bool ok = ops->build ? ops->build(comp, ovr, data) : true;
            if (!ok) {
                free(data);
                data = NULL;
            } else {
                built->built_mask |= (1ull << comp->id);
            }
            built->component_data[comp->id] = data;
        } else {
            bool ok = ops->build ? ops->build(comp, ovr, NULL) : true;
            if (ok) {
                built->built_mask |= (1ull << comp->id);
            }
        }
    }
}

static void ensure_component_slot(pf_loading_built_entity_t* built, ComponentEnum id)
{
    if (built->component_data[id]) return;
    const pf_component_ops_t* ops = pf_register_get(id);
    if (!ops || ops->component_size == 0) {
        LOGC(LOGCAT_PREFAB, LOG_LVL_FATAL, "prefab: missing data slot for component %d", (int)id);
        abort();
    }
    built->component_data[id] = calloc(1, ops->component_size);
    if (!built->component_data[id]) {
        LOGC(LOGCAT_PREFAB, LOG_LVL_FATAL, "prefab: allocation failed for component %d", (int)id);
        abort();
    }
    built->built_mask |= (1ull << id);
}

static void pf_loading_apply_overrides(pf_loading_built_entity_t* built, const pf_override_ctx_t* ovr)
{
    if (!built || !ovr || !ovr->enabled) return;

    for (int i = 0; i < ENUM_COMPONENT_COUNT; ++i) {
        const pf_component_ops_t* ops = pf_register_get((ComponentEnum)i);
        if (!ops || !ops->override) continue;

        const bool has_component = (built->built_mask & (1ull << i)) != 0;
        const bool override_flag = (built->override_mask & (1ull << i)) != 0;
        const bool override_missing = (!has_component && ops->override_if_missing);
        if (!override_flag && !override_missing) continue;

        ensure_component_slot(built, (ComponentEnum)i);
        ops->override(ovr, built->component_data[i]);
    }
}

static void pf_loading_add_to_ecs(ecs_entity_t e, const pf_loading_built_entity_t* built)
{
    if (!built) return;

    for (int i = 0; i < ENUM_COMPONENT_COUNT; ++i) {
        if ((built->built_mask & (1ull << i)) == 0) continue;
        const pf_component_ops_t* ops = pf_register_get((ComponentEnum)i);
        if (!ops || !ops->apply) {
            LOGC(LOGCAT_PREFAB, LOG_LVL_FATAL, "prefab: missing apply for component %d", i);
            abort();
        }
        ops->apply(e, built->component_data[i]);
    }

    if ((built->present_mask & CMP_SPR) != 0 && (built->built_mask & CMP_SPR) == 0) {
        LOGC(LOGCAT_PREFAB, LOG_LVL_WARN, "prefab spr missing path");
    }
}

ecs_entity_t pf_spawn_entity(const prefab_t* prefab, const tiled_object_t* obj)
{
    ecs_entity_t e = ecs_create();
    if (!prefab) return e;

    const pf_override_ctx_t ovr = {
        .obj = obj,
        .enabled = obj != NULL,
    };

    // Phase 1: build all components into plain structs (no ECS).
    pf_loading_built_entity_t built = {0};
    pf_loading_build_entity_components(&built, prefab, &ovr);

    // Phase 2: apply overrides to the built data (e.g., tiled object properties).
    pf_loading_apply_overrides(&built, &ovr);

    // Phase 3: add built components to ECS.
    pf_loading_add_to_ecs(e, &built);

    pf_loading_built_entity_free(&built);

    return e;
}

ecs_entity_t pf_spawn_entity_from_path(const char* prefab_path, const tiled_object_t* obj)
{
    prefab_t prefab;
    if (!prefab_load(prefab_path, &prefab)) {
        LOGC(LOGCAT_PREFAB, LOG_LVL_ERROR, "prefab: could not load %s", prefab_path ? prefab_path : "(null)");
        return ecs_null();
    }
    ecs_entity_t e = pf_spawn_entity(&prefab, obj);
    prefab_free(&prefab);
    return e;
}

static char* join_relative_path(const char* base_path, const char* rel)
{
    if (!rel || rel[0] == '\0') return NULL;
    if (!base_path || rel[0] == '/' || rel[0] == '\\') return xstrdup_local(rel);
    const char* slash = strrchr(base_path, '/');
#ifdef _WIN32
    const char* bslash = strrchr(base_path, '\\');
    if (!slash || (bslash && bslash > slash)) slash = bslash;
#endif
    if (!slash) return xstrdup_local(rel);
    size_t dir_len = (size_t)(slash - base_path) + 1;
    size_t rel_len = strlen(rel);
    char* buf = (char*)malloc(dir_len + rel_len + 1);
    if (!buf) return NULL;
    memcpy(buf, base_path, dir_len);
    memcpy(buf + dir_len, rel, rel_len);
    buf[dir_len + rel_len] = '\0';
    return buf;
}

size_t pf_spawn_from_map(const world_map_t* map, const char* tmx_path)
{
    if (!map) return 0;

    size_t spawned = 0;
    for (size_t i = 0; i < map->object_count; ++i) {
        const tiled_object_t* obj = &map->objects[i];
        const char* prefab_rel = tiled_object_get_property_value(obj, "entityprefab");
        if (!prefab_rel) continue;

        char* resolved = join_relative_path(tmx_path, prefab_rel);
        const char* path = resolved ? resolved : prefab_rel;
        ecs_entity_t ent = pf_spawn_entity_from_path(path, obj);
        if (resolved) free(resolved);
        int idx = ent_index_checked(ent);
        if (idx >= 0) spawned++;
    }

    return spawned;
}
