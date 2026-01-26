#include "engine/prefab/parser/prefab.h"
#include "shared/components_meta.h"
#include "engine/core/logger/logger.h"
#include "xml.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static void free_component(prefab_component_t* c);

// Duplicate a C string
static char* pf_xstrdup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char* p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static bool pf_parse_bool_str(const char* s)
{
    if (!s) return false;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return false;
    return (strcasecmp(s, "1") == 0) ||
           (strcasecmp(s, "true") == 0) ||
           (strcasecmp(s, "yes") == 0) ||
           (strcasecmp(s, "on") == 0);
}

// Duplicate an xml_string into a null-terminated C string
static char* xml_string_dup_local(struct xml_string* xs) {
    if (!xs) return NULL;
    size_t len = xml_string_length(xs);
    char* buf = (char*)malloc(len + 1);
    if (!buf) return NULL;
    xml_string_copy(xs, (uint8_t*)buf, len);
    buf[len] = '\0';
    return buf;
}

// Load and parse an XML document from a file path
static struct xml_document* load_xml_document(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long len = ftell(f);
    if (len < 0) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }

    size_t n = (size_t)len;
    char* buf = (char*)malloc(n + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t read = fread(buf, 1, n, f);
    fclose(f);
    buf[read] = '\0';
    n = read;

    size_t offset = 0;
    if (n >= 3 && (unsigned char)buf[0] == 0xEF && (unsigned char)buf[1] == 0xBB && (unsigned char)buf[2] == 0xBF) {
        offset = 3;
    }
    if (n > offset + 5 && strncmp(buf + offset, "<?xml", 5) == 0) {
        char* pi_end = strstr(buf + offset, "?>");
        if (pi_end) {
            offset = (size_t)((pi_end - buf) + 2);
        }
    }

    size_t trimmed = (offset <= n) ? (n - offset) : 0;
    if (trimmed == 0) {
        free(buf);
        return NULL;
    }
    char* clean = (char*)malloc(trimmed);
    if (!clean) {
        free(buf);
        return NULL;
    }
    memcpy(clean, buf + offset, trimmed);
    free(buf);

    return xml_parse_document((uint8_t*)clean, trimmed);
}

// Get a duplicated attribute value by name from an XML node
static char* node_attr_strdup_local(struct xml_node* node, const char* name) {
    if (!node || !name) return NULL;
    size_t attr_count = xml_node_attributes(node);
    for (size_t i = 0; i < attr_count; ++i) {
        struct xml_string* an = xml_node_attribute_name(node, i);
        if (!an) continue;
        size_t len = xml_string_length(an);
        char* tmp = (char*)malloc(len + 1);
        if (!tmp) return NULL;
        xml_string_copy(an, (uint8_t*)tmp, len);
        tmp[len] = '\0';
        bool match = (strcmp(tmp, name) == 0);
        if (match) {
            struct xml_string* av = xml_node_attribute_content(node, i);
            char* out = xml_string_dup_local(av);
            free(tmp);
            return out;
        }
        free(tmp);
    }
    return NULL;
}

// Check if an XML node's name matches a given local name
static bool node_name_is_local(struct xml_node* node, const char* name) {
    struct xml_string* ns = xml_node_name(node);
    if (!ns || !name) return false;
    size_t len = xml_string_length(ns);
    char* tmp = (char*)malloc(len + 1);
    if (!tmp) return false;
    xml_string_copy(ns, (uint8_t*)tmp, len);
    tmp[len] = '\0';
    bool eq = strcmp(tmp, name) == 0;
    free(tmp);
    return eq;
}

typedef struct {
    char* data;
    size_t len;
    size_t cap;
} strbuf_t;

static bool sb_reserve(strbuf_t* sb, size_t add)
{
    size_t need = sb->len + add + 1;
    if (need <= sb->cap) return true;
    size_t new_cap = sb->cap ? sb->cap * 2 : 128;
    while (new_cap < need) new_cap *= 2;
    char* tmp = (char*)realloc(sb->data, new_cap);
    if (!tmp) return false;
    sb->data = tmp;
    sb->cap = new_cap;
    return true;
}

static bool sb_append_n(strbuf_t* sb, const char* s, size_t n)
{
    if (!sb || !s) return false;
    if (!sb_reserve(sb, n)) return false;
    memcpy(sb->data + sb->len, s, n);
    sb->len += n;
    sb->data[sb->len] = '\0';
    return true;
}

static bool sb_append(strbuf_t* sb, const char* s)
{
    return sb_append_n(sb, s, strlen(s));
}

static bool sb_append_char(strbuf_t* sb, char c)
{
    if (!sb_reserve(sb, 1)) return false;
    sb->data[sb->len++] = c;
    sb->data[sb->len] = '\0';
    return true;
}

static bool sb_append_escaped(strbuf_t* sb, const char* s, bool escape_quotes)
{
    if (!s) return true;
    for (const char* p = s; *p; ++p) {
        const char* rep = NULL;
        char ch = *p;
        switch (ch) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = escape_quotes ? "&quot;" : NULL; break;
            case '\'': rep = escape_quotes ? "&apos;" : NULL; break;
            default: break;
        }
        if (rep) {
            if (!sb_append(sb, rep)) return false;
        } else {
            if (!sb_append_char(sb, ch)) return false;
        }
    }
    return true;
}

static bool xml_node_to_string_rec(struct xml_node* node, strbuf_t* sb)
{
    char* name = xml_string_dup_local(xml_node_name(node));
    if (!name) return false;

    if (!sb_append_char(sb, '<') || !sb_append(sb, name)) {
        free(name);
        return false;
    }

    size_t attr_count = xml_node_attributes(node);
    for (size_t i = 0; i < attr_count; ++i) {
        char* an = xml_string_dup_local(xml_node_attribute_name(node, i));
        char* av = xml_string_dup_local(xml_node_attribute_content(node, i));
        if (!an) {
            free(av);
            continue;
        }
        if (!sb_append_char(sb, ' ') || !sb_append(sb, an) || !sb_append(sb, "=\"") ||
            !sb_append_escaped(sb, av ? av : "", true) || !sb_append_char(sb, '"')) {
            free(an);
            free(av);
            free(name);
            return false;
        }
        free(an);
        free(av);
    }

    size_t child_count = xml_node_children(node);
    struct xml_string* content = xml_node_content(node);
    size_t content_len = content ? xml_string_length(content) : 0;
    bool has_content = content_len > 0;

    if (child_count == 0 && !has_content) {
        if (!sb_append(sb, " />")) {
            free(name);
            return false;
        }
    } else {
        if (!sb_append_char(sb, '>')) {
            free(name);
            return false;
        }
        if (has_content) {
            char* text = xml_string_dup_local(content);
            if (!sb_append_escaped(sb, text ? text : "", false)) {
                free(text);
                free(name);
                return false;
            }
            free(text);
        }
        for (size_t i = 0; i < child_count; ++i) {
            struct xml_node* child = xml_node_child(node, i);
            if (!xml_node_to_string_rec(child, sb)) {
                free(name);
                return false;
            }
        }
        if (!sb_append(sb, "</") || !sb_append(sb, name) || !sb_append_char(sb, '>')) {
            free(name);
            return false;
        }
    }

    free(name);
    return true;
}

static char* xml_node_to_string(struct xml_node* node)
{
    if (!node) return NULL;
    strbuf_t sb = {0};
    if (!xml_node_to_string_rec(node, &sb)) {
        free(sb.data);
        return NULL;
    }
    return sb.data;
}

// Add a property key-value pair to a prefab component
static void add_prop(prefab_component_t* comp, char* name, char* value) {
    if (!comp || !name) {
        free(name);
        free(value);
        return;
    }
    size_t idx = comp->prop_count;
    prefab_kv_t* tmp = (prefab_kv_t*)realloc(comp->props, (idx + 1) * sizeof(prefab_kv_t));
    if (!tmp) {
        free(name);
        free(value);
        return;
    }
    comp->props = tmp;
    comp->props[idx].name = name;
    comp->props[idx].value = value ? value : pf_xstrdup("");
    comp->prop_count = idx + 1;
}

// Parse a component XML node into a prefab component structure
static bool parse_component_node(struct xml_node* comp_node, prefab_component_t* out) {
    memset(out, 0, sizeof(*out));
    char* type = node_attr_strdup_local(comp_node, "type");
    ComponentEnum id;
    if (!type || !component_id_from_string(type, &id)) {
        LOGC(LOGCAT_PREFAB, LOG_LVL_ERROR, "prefab: unknown component type");
        free(type);
        return false;
    }
    out->id = id;
    out->type_name = type;
    out->xml = xml_node_to_string(comp_node);
    if (!out->xml) {
        free_component(out);
        return false;
    }

    size_t attr_count = xml_node_attributes(comp_node);
    for (size_t i = 0; i < attr_count; ++i) {
        struct xml_string* an = xml_node_attribute_name(comp_node, i);
        struct xml_string* av = xml_node_attribute_content(comp_node, i);
        char* n = xml_string_dup_local(an);
        if (!n) continue;
        if (strcasecmp(n, "type") == 0) {
            free(n);
            continue;
        }
        if (strcasecmp(n, "override") == 0) {
            char* v = xml_string_dup_local(av);
            out->override_after_spawn = pf_parse_bool_str(v);
            free(v);
            free(n);
            continue;
        }
        char* v = xml_string_dup_local(av);
        add_prop(out, n, v);
    }

    size_t child_count = xml_node_children(comp_node);
    for (size_t i = 0; i < child_count; ++i) {
        struct xml_node* child = xml_node_child(comp_node, i);
        if (node_name_is_local(child, "property")) {
            char* pname = node_attr_strdup_local(child, "name");
            if (!pname) continue;
            char* pval = node_attr_strdup_local(child, "value");
            if (!pval) {
                struct xml_string* pv = xml_node_content(child);
                pval = xml_string_dup_local(pv);
            }
            add_prop(out, pname, pval ? pval : pf_xstrdup(""));
        }
    }

    return true;
}

// Free memory allocated for a prefab component
static void free_component(prefab_component_t* c) {
    if (!c) return;
    free(c->type_name);
    free(c->xml);
    for (size_t p = 0; p < c->prop_count; ++p) {
        free(c->props[p].name);
        free(c->props[p].value);
    }
    free(c->props);
    // dono why i have to do this
    *c = (prefab_component_t){ .id = 0 };
}

bool prefab_load(const char* path, prefab_t* out_prefab) {
    if (!path || !out_prefab) return false;
    *out_prefab = (prefab_t){ .name = NULL };

    struct xml_document* doc = load_xml_document(path);
    if (!doc) {
        LOGC(LOGCAT_PREFAB, LOG_LVL_ERROR, "prefab: failed to read %s", path);
        return false;
    }
    struct xml_node* root = xml_document_root(doc);
    if (!root || !node_name_is_local(root, "prefab")) {
        LOGC(LOGCAT_PREFAB, LOG_LVL_ERROR, "prefab: root must be <prefab> in %s", path);
        xml_document_free(doc, true);
        return false;
    }

    out_prefab->name = node_attr_strdup_local(root, "name");

    size_t children = xml_node_children(root);
    for (size_t i = 0; i < children; ++i) {
        struct xml_node* child = xml_node_child(root, i);
        if (!node_name_is_local(child, "component")) continue;

        prefab_component_t comp = { .id = 0 };
        if (!parse_component_node(child, &comp)) {
            free_component(&comp);
            xml_document_free(doc, true);
            prefab_free(out_prefab);
            return false;
        }
        prefab_component_t* tmp = (prefab_component_t*)realloc(out_prefab->components, (out_prefab->component_count + 1) * sizeof(prefab_component_t));
        if (!tmp) {
            free_component(&comp);
            prefab_free(out_prefab);
            xml_document_free(doc, true);
            return false;
        }
        out_prefab->components = tmp;
        out_prefab->components[out_prefab->component_count++] = comp;
    }

    xml_document_free(doc, true);
    return true;
}

// Free memory allocated for a prefab
void prefab_free(prefab_t* prefab) {
    if (!prefab) return;
    free(prefab->name);
    //Free each component
    if (prefab->components) {
        for (size_t i = 0; i < prefab->component_count; ++i) {
            free_component(&prefab->components[i]);
        }
    }
    free(prefab->components);
    *prefab = (prefab_t){ .name = NULL };
}
