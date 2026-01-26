#include "engine/ecs/ecs_physics.h"

#include "engine/core/logger/logger.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct {
    char* name;
    unsigned int bit;
} phys_tag_entry_t;

static phys_tag_entry_t g_tags[32];
static int g_tag_count = 0;

static char* phys_tag_strdup(const char* s)
{
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
    return out;
}

void phys_tag_reset_registry(void)
{
    for (int i = 0; i < g_tag_count; ++i) {
        free(g_tags[i].name);
        g_tags[i] = (phys_tag_entry_t){ .name = NULL };
    }
    g_tag_count = 0;
}

static const char* trim_left(const char* s)
{
    if (!s) return NULL;
    while (*s && isspace((unsigned char)*s)) {
        ++s;
    }
    return s;
}

static char* trim_copy(const char* s, size_t len)
{
    while (len > 0 && isspace((unsigned char)s[0])) {
        s++;
        len--;
    }
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        len--;
    }
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
    return out;
}

unsigned int phys_tag_bit(const char* name)
{
    name = trim_left(name);
    if (!name || !*name) return 0u;

    for (int i = 0; i < g_tag_count; ++i) {
        if (strcasecmp(name, g_tags[i].name) == 0) {
            return g_tags[i].bit;
        }
    }

    if (g_tag_count >= (int)(sizeof(g_tags) / sizeof(g_tags[0]))) {
        LOGC(LOGCAT_ECS, LOG_LVL_WARN, "phys tags: registry full, dropping '%s'", name);
        return 0u;
    }

    const unsigned int bit = 1u << g_tag_count;
    g_tags[g_tag_count].name = phys_tag_strdup(name);
    if (!g_tags[g_tag_count].name) {
        return 0u;
    }
    g_tags[g_tag_count].bit = bit;
    g_tag_count++;
    return bit;
}

unsigned int phys_parse_tag_list(const char* s)
{
    if (!s) return 0u;
    unsigned int bits = 0u;
    const char* p = s;

    while (*p) {
        while (*p && (isspace((unsigned char)*p) || *p == '|' || *p == ',')) {
            ++p;
        }
        const char* start = p;
        while (*p && *p != '|' && *p != ',') {
            ++p;
        }
        if (p == start) continue;
        char* token = trim_copy(start, (size_t)(p - start));
        if (!token) continue;

        char* name = token;
        bool negate = false;
        if (name[0] == '!' || name[0] == '~') {
            negate = true;
            name++;
            while (*name && isspace((unsigned char)*name)) {
                name++;
            }
        }

        if (name[0] != '\0') {
            if (strcasecmp(name, "all") == 0) {
                bits = negate ? 0u : 0xFFFFFFFFu;
                free(token);
                continue;
            }
            if (strcasecmp(name, "none") == 0) {
                if (!negate) bits = 0u;
                free(token);
                continue;
            }
            unsigned int bit = phys_tag_bit(name);
            if (negate) {
                if (bits == 0u) bits = 0xFFFFFFFFu;
                bits &= ~bit;
            } else {
                bits |= bit;
            }
        }
        free(token);
    }

    return bits;
}
