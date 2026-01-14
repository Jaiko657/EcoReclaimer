#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <strings.h>

typedef enum {
#define X(name) ENUM_##name,
#include "engine/ecs/components_engine.def"
#include "game/components/components_game.def"
#undef X
    ENUM_COMPONENT_COUNT
} ComponentEnum;

typedef uint64_t ComponentMask;

enum {
#define X(name) CMP_##name = 1ull << ENUM_##name,
#include "engine/ecs/components_engine.def"
#include "game/components/components_game.def"
#undef X
};

typedef struct {
    const char* name;
    ComponentEnum id;
    ComponentMask mask;
} component_meta_t;

static const component_meta_t k_component_meta[] = {
#define X(name) { #name, ENUM_##name, CMP_##name },
#include "engine/ecs/components_engine.def"
#include "game/components/components_game.def"
#undef X
};

static inline const char* component_name_from_id(ComponentEnum id)
{
    for (size_t i = 0; i < sizeof(k_component_meta) / sizeof(k_component_meta[0]); ++i) {
        if (k_component_meta[i].id == id) return k_component_meta[i].name;
    }
    return NULL;
}

static inline bool component_id_from_string(const char* s, ComponentEnum* out_id)
{
    if (!s || !out_id) return false;
    for (size_t i = 0; i < sizeof(k_component_meta) / sizeof(k_component_meta[0]); ++i) {
        if (strcasecmp(k_component_meta[i].name, s) == 0) {
            *out_id = k_component_meta[i].id;
            return true;
        }
    }
    return false;
}

static inline bool component_mask_from_strn(const char* s, size_t len, ComponentMask* out_mask)
{
    if (!s || !out_mask || len == 0) return false;
    for (size_t i = 0; i < sizeof(k_component_meta) / sizeof(k_component_meta[0]); ++i) {
        const char* name = k_component_meta[i].name;
        if (!name) continue;
        if (strncasecmp(name, s, len) == 0 && name[len] == '\0') {
            *out_mask = k_component_meta[i].mask;
            return true;
        }
    }
    return false;
}
