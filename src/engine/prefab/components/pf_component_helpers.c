#include "engine/prefab/components/pf_component_helpers.h"
#include "shared/components_meta.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// Parses a component mask from a string.
ComponentMask pf_parse_mask(const char* s, bool* out_ok)
{
    if (out_ok) *out_ok = false;
    if (!s) return 0;
    ComponentMask mask = 0;
    const char* p = s;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        const char* start = p;
        while (*p && *p != '|' && *p != ',' && !isspace((unsigned char)*p)) p++;
        size_t len = (size_t)(p - start);
        if (len > 0) {
            ComponentMask part = 0;
            if (component_mask_from_strn(start, len, &part)) {
                mask |= part;
                if (out_ok) *out_ok = true;
            }
        }
        while (*p && (*p == '|' || *p == ',' || isspace((unsigned char)*p))) p++;
    }
    if (!mask && s && isdigit((unsigned char)*s)) {
        mask = (ComponentMask)strtoull(s, NULL, 0);
        if (out_ok) *out_ok = true;
    }
    return mask;
}

// Parses a float from a string. Returns true on success.
bool pf_parse_float(const char* s, float* out_v)
{
    if (!s || !out_v) return false;
    char* end = NULL;
    float v = strtof(s, &end);
    if (end == s) return false;
    *out_v = v;
    return true;
}

// Parses an int from a string. Returns true on success.
bool pf_parse_int(const char* s, int* out_v)
{
    if (!s || !out_v) return false;
    char* end = NULL;
    int v = (int)strtol(s, &end, 0);
    if (end == s) return false;
    *out_v = v;
    return true;
}

// Parses a facing_t from a string. Returns the fallback value on failure.
facing_t pf_parse_facing(const char* s, facing_t fallback)
{
    if (!s) return fallback;
    if (strcasecmp(s, "n") == 0 || strcasecmp(s, "north") == 0) return DIR_NORTH;
    if (strcasecmp(s, "ne") == 0 || strcasecmp(s, "northeast") == 0) return DIR_NORTHEAST;
    if (strcasecmp(s, "e") == 0 || strcasecmp(s, "east") == 0) return DIR_EAST;
    if (strcasecmp(s, "se") == 0 || strcasecmp(s, "southeast") == 0) return DIR_SOUTHEAST;
    if (strcasecmp(s, "s") == 0 || strcasecmp(s, "south") == 0) return DIR_SOUTH;
    if (strcasecmp(s, "sw") == 0 || strcasecmp(s, "southwest") == 0) return DIR_SOUTHWEST;
    if (strcasecmp(s, "w") == 0 || strcasecmp(s, "west") == 0) return DIR_WEST;
    if (strcasecmp(s, "nw") == 0 || strcasecmp(s, "northwest") == 0) return DIR_NORTHWEST;
    return fallback;
}

// Finds a property value in a prefab component by name.
const char* pf_find_prop(const prefab_component_t* comp, const char* field)
{
    if (!comp || !field) return NULL;
    for (size_t i = 0; i < comp->prop_count; ++i) {
        if (comp->props[i].name && strcasecmp(comp->props[i].name, field) == 0) {
            return comp->props[i].value;
        }
    }
    return NULL;
}

// Gets an overridden property value from a tiled object, considering the prefab component's type name.
const char* pf_override_value(const prefab_component_t* comp, const pf_override_ctx_t* ovr, const char* field)
{
    const tiled_object_t* obj = pf_override_obj(ovr);
    if (!obj || !field) return NULL;
    char key[128];
    if (comp && comp->type_name) {
        snprintf(key, sizeof(key), "%s.%s", comp->type_name, field);
        const char* v = tiled_object_get_property_value(obj, key);
        if (v) return v;
    }
    return tiled_object_get_property_value(obj, field);
}

// Gets the combined property value, checking for overrides in the tiled object first.
const char* pf_combined_value(const prefab_component_t* comp, const pf_override_ctx_t* ovr, const char* field)
{
    const char* override = pf_override_value(comp, ovr, field);
    if (override) return override;
    return pf_find_prop(comp, field);
}

// Computes the default position for a tiled object.
v2f pf_object_position_default(const tiled_object_t* obj)
{
    if (!obj) return v2f_make(0.0f, 0.0f);
    float x = obj->x;
    float y = obj->y;
    bool is_tile_obj = obj->gid != 0;
    if (obj->w > 0.0f || obj->h > 0.0f) {
        x += obj->w * 0.5f;
        y += is_tile_obj ? -obj->h * 0.5f : obj->h * 0.5f;
    }
    return v2f_make(x, y);
}

// Looks up an object property using "COMP.field" then plain "field".
const char* pf_object_prop_only(const pf_override_ctx_t* ovr, const char* comp_name, const char* field)
{
    const tiled_object_t* obj = pf_override_obj(ovr);
    if (!obj || !field || !comp_name) return NULL;
    char key[128];
    snprintf(key, sizeof(key), "%s.%s", comp_name, field);
    const char* v = tiled_object_get_property_value(obj, key);
    if (v) return v;
    return tiled_object_get_property_value(obj, field);
}
