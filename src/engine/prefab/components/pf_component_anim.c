#include "engine/prefab/pf_components_engine.h"
#include "engine/ecs/ecs.h"
#include "xml.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    anim_frame_coord_t* frames;
    size_t frame_count;
} anim_seq_tmp_t;

typedef struct {
    anim_seq_tmp_t* seqs;
    size_t seq_count;
} anim_def_tmp_t;

static char* xml_string_dup_local(struct xml_string* xs)
{
    if (!xs) return NULL;
    size_t len = xml_string_length(xs);
    char* buf = (char*)malloc(len + 1);
    if (!buf) return NULL;
    xml_string_copy(xs, (uint8_t*)buf, len);
    buf[len] = '\0';
    return buf;
}

static bool node_name_is_local(struct xml_node* node, const char* name)
{
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

static char* node_attr_strdup_local(struct xml_node* node, const char* name)
{
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

static void anim_def_tmp_free(anim_def_tmp_t* def)
{
    if (!def) return;
    for (size_t i = 0; i < def->seq_count; ++i) {
        free(def->seqs[i].frames);
    }
    free(def->seqs);
    *def = (anim_def_tmp_t){0};
}

static bool anim_def_add_seq(anim_def_tmp_t* def, anim_seq_tmp_t seq)
{
    anim_seq_tmp_t* tmp = (anim_seq_tmp_t*)realloc(def->seqs, (def->seq_count + 1) * sizeof(*def->seqs));
    if (!tmp) return false;
    def->seqs = tmp;
    def->seqs[def->seq_count++] = seq;
    return true;
}

static bool parse_anim_sequence(struct xml_node* anim_node, anim_def_tmp_t* def)
{
    if (!def) return false;

    size_t child_count = xml_node_children(anim_node);
    size_t frame_count = 0;
    for (size_t i = 0; i < child_count; ++i) {
        struct xml_node* frame_node = xml_node_child(anim_node, i);
        if (node_name_is_local(frame_node, "frame")) frame_count++;
    }

    anim_seq_tmp_t seq = {0};
    if (frame_count > 0) {
        seq.frames = (anim_frame_coord_t*)malloc(frame_count * sizeof(anim_frame_coord_t));
        if (!seq.frames) return false;
    }

    size_t fi = 0;
    for (size_t i = 0; i < child_count && fi < frame_count; ++i) {
        struct xml_node* frame_node = xml_node_child(anim_node, i);
        if (!node_name_is_local(frame_node, "frame")) continue;
        int col = 0, row = 0;
        char* scol = node_attr_strdup_local(frame_node, "col");
        char* srow = node_attr_strdup_local(frame_node, "row");
        if (scol) { col = atoi(scol); free(scol); }
        if (srow) { row = atoi(srow); free(srow); }
        seq.frames[fi++] = (anim_frame_coord_t){ (uint8_t)col, (uint8_t)row };
    }
    seq.frame_count = fi;

    if (!anim_def_add_seq(def, seq)) {
        free(seq.frames);
        return false;
    }
    return true;
}

static bool parse_anim_component_xml(const char* xml, anim_def_tmp_t* out_def)
{
    if (!xml || !out_def) return false;
    *out_def = (anim_def_tmp_t){0};

    size_t len = strlen(xml);
    char* buf = (char*)malloc(len + 1);
    if (!buf) return false;
    memcpy(buf, xml, len + 1);

    struct xml_document* doc = xml_parse_document((uint8_t*)buf, len);
    if (!doc) {
        free(buf);
        return false;
    }

    struct xml_node* root = xml_document_root(doc);
    if (!root || !node_name_is_local(root, "component")) {
        xml_document_free(doc, true);
        return false;
    }

    size_t child_count = xml_node_children(root);
    for (size_t i = 0; i < child_count; ++i) {
        struct xml_node* child = xml_node_child(root, i);
        if (!node_name_is_local(child, "anim")) continue;
        if (!parse_anim_sequence(child, out_def)) {
            anim_def_tmp_free(out_def);
            xml_document_free(doc, true);
            return false;
        }
    }

    xml_document_free(doc, true);
    return true;
}

void pf_component_anim_free(pf_component_anim_t* anim)
{
    if (!anim) return;
    free(anim->frames_per_anim);
    free(anim->frames);
    memset(anim, 0, sizeof(*anim));
}

bool pf_component_anim_build(const prefab_component_t* comp, const pf_override_ctx_t* ovr, pf_component_anim_t* out_anim)
{
    if (!out_anim) return false;
    if (!comp || !comp->xml) return false;

    memset(out_anim, 0, sizeof(*out_anim));
    int frame_w = 0;
    int frame_h = 0;
    float fps = 0.0f;
    pf_parse_int(pf_combined_value(comp, ovr, "frame_w"), &frame_w);
    pf_parse_int(pf_combined_value(comp, ovr, "frame_h"), &frame_h);
    pf_parse_float(pf_combined_value(comp, ovr, "fps"), &fps);

    anim_def_tmp_t def = {0};
    if (!parse_anim_component_xml(comp->xml, &def)) {
        return false;
    }

    int anim_count = (int)def.seq_count;
    if (anim_count < 0) anim_count = 0;
    if (anim_count > MAX_ANIMS) anim_count = MAX_ANIMS;

    out_anim->frame_w = frame_w;
    out_anim->frame_h = frame_h;
    out_anim->fps = fps;
    out_anim->anim_count = anim_count;
    out_anim->frame_buffer_height = anim_count;

    if (anim_count == 0) {
        out_anim->frame_buffer_width = 0;
        anim_def_tmp_free(&def);
        return true;
    }

    int* counts = (int*)malloc((size_t)anim_count * sizeof(int));
    if (!counts) {
        anim_def_tmp_free(&def);
        return false;
    }
    out_anim->frames_per_anim = counts;

    int max_frames = 0;
    for (int i = 0; i < anim_count; ++i) {
        const anim_seq_tmp_t* seq = &def.seqs[i];
        int count = (int)seq->frame_count;
        if (count < 0) count = 0;
        counts[i] = count;
        if (count > max_frames) max_frames = count;
    }

    out_anim->frame_buffer_width = max_frames;

    size_t total_slots = (size_t)anim_count * (size_t)max_frames;
    if (total_slots > 0) {
        anim_frame_coord_t* frames = (anim_frame_coord_t*)malloc(total_slots * sizeof(anim_frame_coord_t));
        if (!frames) {
            pf_component_anim_free(out_anim);
            anim_def_tmp_free(&def);
            return false;
        }
        out_anim->frames = frames;
    }

    for (int i = 0; i < anim_count; ++i) {
        const anim_seq_tmp_t* seq = &def.seqs[i];
        int count = counts[i];
        for (int f = 0; f < count; ++f) {
            anim_frame_coord_t* dst = pf_component_anim_frame_coord_mut(out_anim, i, f);
            if (dst && seq->frames) {
                *dst = seq->frames[f];
            }
        }
    }

    anim_def_tmp_free(&def);
    return true;
}

static void pf_component_anim_apply(ecs_entity_t e, const void* component)
{
    const pf_component_anim_t* anim = (const pf_component_anim_t*)component;
    cmp_add_anim(
        e,
        anim->frame_w,
        anim->frame_h,
        anim->anim_count,
        anim->frames_per_anim,
        anim->frames,
        anim->frame_buffer_width,
        anim->fps);
}

static void pf_component_anim_free_component(void* component)
{
    pf_component_anim_free((pf_component_anim_t*)component);
}

const pf_component_ops_t* pf_component_anim_ops(void)
{
    static const pf_component_ops_t ops = {
        .id = ENUM_ANIM,
        .component_size = sizeof(pf_component_anim_t),
        .build = (pf_build_fn)pf_component_anim_build,
        .apply = pf_component_anim_apply,
        .free = pf_component_anim_free_component,
    };
    return &ops;
}
