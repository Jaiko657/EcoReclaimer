#include "engine/input/input.h"
#include "engine/input/input_backend.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "engine/input/input_tables.h"
#include "engine/core/logger/logger.h"

#ifndef INPUTS_INI_FILENAME
#define INPUTS_INI_FILENAME "inputs.ini"
#endif

#define MAX_KEYS_PER_ACT 6

/*
  Internal storage for action bindings:
  Each logical action can map to up to MAX_KEYS_PER_ACT physical codes.
*/
typedef struct {
    int keys[MAX_KEYS_PER_ACT];
    int key_count;
} action_binding_t;

/*
  Static state that persists across frames.
*/
static action_binding_t g_bindings[ACT_COUNT];
static uint64_t s_prev_down_bits = 0;
static input_t  s_frame_input;
static bool     s_edges_available = false;
static uint64_t s_latched_pressed = 0;
static float    s_latched_wheel   = 0.0f;

/*
  Small helper macro for building bit masks for actions.
*/
#define ACTION_BIT(a) (1ull << (a))

static const char* k_action_names[ACT_COUNT] = {
    [ACT_LEFT] = "ACT_LEFT",
    [ACT_RIGHT] = "ACT_RIGHT",
    [ACT_UP] = "ACT_UP",
    [ACT_DOWN] = "ACT_DOWN",
    [ACT_INTERACT] = "ACT_INTERACT",
    [ACT_LIFT] = "ACT_LIFT",
    [ACT_MOUSE_L] = "ACT_MOUSE_L",
    [ACT_MOUSE_R] = "ACT_MOUSE_R",
#if DEBUG_BUILD
    [ACT_DEBUG_ASSET_PRINT] = "ACT_DEBUG_ASSET_PRINT",
    [ACT_DEBUG_COLLIDER_ECS] = "ACT_DEBUG_COLLIDER_ECS",
    [ACT_DEBUG_COLLIDER_PHYSICS] = "ACT_DEBUG_COLLIDER_PHYSICS",
    [ACT_DEBUG_COLLIDER_STATIC] = "ACT_DEBUG_COLLIDER_STATIC",
    [ACT_DEBUG_TRIGGERS] = "ACT_DEBUG_TRIGGERS",
    [ACT_DEBUG_INSPECT] = "ACT_DEBUG_INSPECT",
    [ACT_DEBUG_RELOAD_TMX] = "ACT_DEBUG_RELOAD_TMX",
    [ACT_DEBUG_FPS] = "ACT_DEBUG_FPS",
    [ACT_DEBUG_TRACE_START] = "ACT_DEBUG_TRACE_START",
    [ACT_DEBUG_TRACE_STOP] = "ACT_DEBUG_TRACE_STOP",
    [ACT_DEBUG_SCREENSHOT] = "ACT_DEBUG_SCREENSHOT",
#endif
};


static int action_id_from_name(const char* name)
{
    if (!name) return -1;
    for (int i = 0; i < ACT_COUNT; ++i) {
        const char* n = k_action_names[i];
        if (n && strcmp(n, name) == 0) return i;
    }
    return -1;
}

#if !DEBUG_BUILD
static bool is_debug_action_name(const char* name)
{
    if (!name) return false;
    return strncmp(name, "ACT_DEBUG_", 10) == 0;
}
#endif

static const char* action_name_from_id(int id)
{
    if (id < 0 || id >= ACT_COUNT) return NULL;
    return k_action_names[id];
}

static char* trim_ws(char* s)
{
    while (*s && isspace((unsigned char)*s)) ++s;
    if (*s == '\0') return s;
    char* end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) --end;
    *end = '\0';
    return s;
}

/*
  Add a physical code to a logical action if there is space.
*/
static void bind_add(action_t a, int code){
    if ((unsigned)a >= ACT_COUNT) return;
    if (g_bindings[a].key_count < MAX_KEYS_PER_ACT) {
        g_bindings[a].keys[g_bindings[a].key_count++] = code;
    }
}

static const char* input_ini_path(void)
{
    return INPUTS_INI_FILENAME;
}

static void input_init_defaults(void){
    memset(g_bindings, 0, sizeof(g_bindings));
    s_prev_down_bits   = 0;
    s_edges_available  = false;

    input_backend_bind_defaults();
}

static void input_save_bindings(void)
{
    FILE* f = fopen(input_ini_path(), "w");
    if (!f) return;
    fprintf(f, "# inputs.ini\n");
    fprintf(f, "# ACT_NAME=KEY_NAME[,KEY_NAME...]\n");
    for (int a = 0; a < ACT_COUNT; ++a) {
        const char* name = action_name_from_id(a);
        if (!name || g_bindings[a].key_count <= 0) continue;
        fprintf(f, "%s=", name);
        for (int i = 0; i < g_bindings[a].key_count; ++i) {
            if (i > 0) fputc(',', f);
            const char* key_name = key_name_from_code(g_bindings[a].keys[i]);
            if (key_name) {
                fputs(key_name, f);
            } else {
                fprintf(f, "%d", g_bindings[a].keys[i]);
            }
        }
        fputc('\n', f);
    }
    fclose(f);
}

static bool input_load_bindings(void)
{
    FILE* f = fopen(input_ini_path(), "r");
    if (!f) return false;

    memset(g_bindings, 0, sizeof(g_bindings));

    int line_no = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        ++line_no;
        char* p = trim_ws(line);
        if (*p == '\0' || *p == '#') continue;

        char* eq = strchr(p, '=');
        if (!eq) {
            LOGC(LOGCAT(INPUT), LOG_LVL_FATAL, "inputs.ini:%d missing '='", line_no);
            abort();
        }
        *eq = '\0';
        char* name = trim_ws(p);
        int act = action_id_from_name(name);
        if (act < 0) {
#if !DEBUG_BUILD
            if (is_debug_action_name(name)) {
                continue;
            }
#endif
            LOGC(LOGCAT(INPUT), LOG_LVL_FATAL, "inputs.ini:%d unknown action '%s'", line_no, name);
            abort();
        }

        char* cur = trim_ws(eq + 1);
        char* tok = cur;
        while (tok && *tok) {
            char* comma = strchr(tok, ',');
            if (comma) *comma = '\0';
            char* key_str = trim_ws(tok);
            int key = 0;
            if (key_code_from_name(key_str, &key)) {
                bind_add(act, key);
            } else {
                char* endp = NULL;
                key = (int)strtol(key_str, &endp, 10);
                if (!endp || *trim_ws(endp) != '\0') {
                    LOGC(LOGCAT(INPUT), LOG_LVL_FATAL,
                         "inputs.ini:%d unknown key '%s' for %s", line_no, key_str, name);
                    abort();
                }
                bind_add(act, key);
            }
            if (!comma) break;
            tok = comma + 1;
        }
    }

    if (ferror(f)) {
        LOGC(LOGCAT(INPUT), LOG_LVL_FATAL, "inputs.ini: read error");
        abort();
    }
    fclose(f);
    return true;
}

static bool input_has_any_binding(void)
{
    for (int a = 0; a < ACT_COUNT; ++a) {
        if (g_bindings[a].key_count > 0) return true;
    }
    return false;
}

void input_init(void)
{
    s_prev_down_bits   = 0;
    s_edges_available  = false;
    if (!input_load_bindings() || !input_has_any_binding()) {
        input_init_defaults();
        input_save_bindings();
    }
}

/*
  Public binding API: attach a Raylib key/mouse code to a logical action.
*/
void input_bind(action_t act, int keycode){
    bind_add(act, keycode);
}

/*
  Poll the OS once per render frame and compute:
  - 'down' bitset for actions that are held
  - 'pressed' bitset for rising edges (including Raylib's own edge helpers)
  Also capture mouse position and wheel delta, and derive a normalized move axis.
*/
void input_begin_frame(void){
    input_t in;
    memset(&in, 0, sizeof(in));

    uint64_t down_bits = 0;
    for (int a = 0; a < ACT_COUNT; ++a){
        for (int i = 0; i < g_bindings[a].key_count; ++i){
            if (input_backend_is_down(g_bindings[a].keys[i])) { down_bits |= ACTION_BIT(a); break; }
        }
    }

    // edges seen THIS render frame
    uint64_t pressed_now = down_bits & ~s_prev_down_bits;
    for (int a = 0; a < ACT_COUNT; ++a){
        bool any_pressed = false;
        for (int i = 0; i < g_bindings[a].key_count; ++i){
            if (input_backend_is_pressed(g_bindings[a].keys[i])) { any_pressed = true; break; }
        }
        if (any_pressed) pressed_now |= ACTION_BIT(a);
    }

    // LATCH edges + wheel until a fixed tick consumes them
    s_latched_pressed |= pressed_now;
    s_latched_wheel   += input_backend_mouse_wheel();

    s_prev_down_bits = down_bits;

    input_vec2 m = input_backend_mouse_pos();
    in.down    = down_bits;
    in.pressed = s_latched_pressed;   // <- latched
    in.mouse.x = m.x;
    in.mouse.y = m.y;
    in.mouse_wheel = s_latched_wheel; // <- latched

    in.moveX = ((down_bits & ACTION_BIT(ACT_RIGHT)) ? 1.f : 0.f)
             - ((down_bits & ACTION_BIT(ACT_LEFT )) ? 1.f : 0.f);
    in.moveY = ((down_bits & ACTION_BIT(ACT_DOWN )) ? 1.f : 0.f)
             - ((down_bits & ACTION_BIT(ACT_UP   )) ? 1.f : 0.f);
    float mag = sqrtf(in.moveX*in.moveX + in.moveY*in.moveY);
    if (mag > 0.f){ in.moveX /= mag; in.moveY /= mag; }

    s_frame_input     = in;
    s_edges_available = true;
}

/*
  Provide the latched snapshot to a fixed tick.
  Only the first tick after input_begin_frame() gets 'pressed' edges and wheel delta;
  following ticks for the same frame get those cleared.
*/
input_t input_for_tick(void){
    input_t out = s_frame_input;
    if (!s_edges_available){
        out.pressed     = 0;
        out.mouse_wheel = 0;
    }
    // First tick after begin_frame consumes the latched edges/wheel
    s_edges_available = false;
    s_latched_pressed = 0;
    s_latched_wheel   = 0.0f;
    return out;
}

const input_t* input_frame_snapshot(void)
{
    return &s_frame_input;
}
