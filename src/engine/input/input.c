#include "engine/input/input.h"
#include "raylib.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "engine/input/input_tables.h"
#include "engine/core/logger.h"

#define MAX_KEYS_PER_BTN 6

/*
  Internal storage for button bindings:
  Each logical button can map to up to MAX_KEYS_PER_BTN physical codes.
*/
typedef struct {
    int keys[MAX_KEYS_PER_BTN];
    int key_count;
} button_binding_t;

/*
  Static state that persists across frames.
*/
static button_binding_t g_bindings[BTN_COUNT];
static uint64_t s_prev_down_bits = 0;
static input_t  s_frame_input;
static bool     s_edges_available = false;
static uint64_t s_latched_pressed = 0;
static float    s_latched_wheel   = 0.0f;

/*
  Small helper macro for building bit masks for buttons.
*/
#define BUTTON_BIT(b) (1ull << (b))

static const char* k_button_names[BTN_COUNT] = {
    [BTN_LEFT] = "BTN_LEFT",
    [BTN_RIGHT] = "BTN_RIGHT",
    [BTN_UP] = "BTN_UP",
    [BTN_DOWN] = "BTN_DOWN",
    [BTN_INTERACT] = "BTN_INTERACT",
    [BTN_LIFT] = "BTN_LIFT",
    [BTN_MOUSE_L] = "BTN_MOUSE_L",
    [BTN_MOUSE_R] = "BTN_MOUSE_R",
#if DEBUG_BUILD
    [BTN_ASSET_DEBUG_PRINT] = "BTN_ASSET_DEBUG_PRINT",
    [BTN_DEBUG_COLLIDER_ECS] = "BTN_DEBUG_COLLIDER_ECS",
    [BTN_DEBUG_COLLIDER_PHYSICS] = "BTN_DEBUG_COLLIDER_PHYSICS",
    [BTN_DEBUG_COLLIDER_STATIC] = "BTN_DEBUG_COLLIDER_STATIC",
    [BTN_DEBUG_TRIGGERS] = "BTN_DEBUG_TRIGGERS",
    [BTN_DEBUG_INSPECT] = "BTN_DEBUG_INSPECT",
    [BTN_DEBUG_RELOAD_TMX] = "BTN_DEBUG_RELOAD_TMX",
    [BTN_DEBUG_FPS] = "BTN_DEBUG_FPS",
    [BTN_DEBUG_TRACE_START] = "BTN_DEBUG_TRACE_START",
    [BTN_DEBUG_TRACE_STOP] = "BTN_DEBUG_TRACE_STOP",
#endif
};


static int button_id_from_name(const char* name)
{
    if (!name) return -1;
    for (int i = 0; i < BTN_COUNT; ++i) {
        const char* n = k_button_names[i];
        if (n && strcmp(n, name) == 0) return i;
    }
    return -1;
}

#if !DEBUG_BUILD
static bool is_debug_button_name(const char* name)
{
    if (!name) return false;
    if (strcmp(name, "BTN_ASSET_DEBUG_PRINT") == 0) return true;
    return strncmp(name, "BTN_DEBUG_", 10) == 0;
}
#endif

static const char* button_name_from_id(int id)
{
    if (id < 0 || id >= BTN_COUNT) return NULL;
    return k_button_names[id];
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
  Add a physical code to a logical button if there is space.
*/
static void bind_add(button_t b, int code){
    if ((unsigned)b >= BTN_COUNT) return;
    if (g_bindings[b].key_count < MAX_KEYS_PER_BTN) {
        g_bindings[b].keys[g_bindings[b].key_count++] = code;
    }
}

static const char* input_ini_path(void)
{
    return "inputs.ini";
}

static void input_init_defaults(void){
    memset(g_bindings, 0, sizeof(g_bindings));
    s_prev_down_bits   = 0;
    s_edges_available  = false;

    /* Movement: Arrows + WASD */
    bind_add(BTN_LEFT,  KEY_LEFT);   bind_add(BTN_LEFT,  KEY_A);
    bind_add(BTN_RIGHT, KEY_RIGHT);  bind_add(BTN_RIGHT, KEY_D);
    bind_add(BTN_UP,    KEY_UP);     bind_add(BTN_UP,    KEY_W);
    bind_add(BTN_DOWN,  KEY_DOWN);   bind_add(BTN_DOWN,  KEY_S);

    /* Interact: E */
    bind_add(BTN_INTERACT, KEY_E);

    /* Lift/Throw: C */
    bind_add(BTN_LIFT, KEY_C);

    /* Mouse buttons */
    bind_add(BTN_MOUSE_L, MOUSE_LEFT_BUTTON);
    bind_add(BTN_MOUSE_R, MOUSE_RIGHT_BUTTON);

#if DEBUG_BUILD
    bind_add(BTN_ASSET_DEBUG_PRINT, KEY_SPACE);

    /* Debug toggles */
    bind_add(BTN_DEBUG_COLLIDER_ECS,     KEY_ONE);
    bind_add(BTN_DEBUG_COLLIDER_PHYSICS, KEY_TWO);
    bind_add(BTN_DEBUG_COLLIDER_STATIC,  KEY_THREE);
    bind_add(BTN_DEBUG_TRIGGERS,         KEY_FOUR);
    bind_add(BTN_DEBUG_INSPECT,          KEY_FIVE);
    bind_add(BTN_DEBUG_RELOAD_TMX,       KEY_R);
    bind_add(BTN_DEBUG_FPS,              KEY_GRAVE);
    bind_add(BTN_DEBUG_TRACE_START,      KEY_F9);
    bind_add(BTN_DEBUG_TRACE_STOP,       KEY_F10);
#endif
}

static void input_save_bindings(void)
{
    FILE* f = fopen(input_ini_path(), "w");
    if (!f) return;
    fprintf(f, "# inputs.ini\n");
    fprintf(f, "# BTN_NAME=KEY_NAME[,KEY_NAME...]\n");
    for (int b = 0; b < BTN_COUNT; ++b) {
        const char* name = button_name_from_id(b);
        if (!name || g_bindings[b].key_count <= 0) continue;
        fprintf(f, "%s=", name);
        for (int i = 0; i < g_bindings[b].key_count; ++i) {
            if (i > 0) fputc(',', f);
            const char* key_name = key_name_from_code(g_bindings[b].keys[i]);
            if (key_name) {
                fputs(key_name, f);
            } else {
                fprintf(f, "%d", g_bindings[b].keys[i]);
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
            LOGC(LOGCAT("INPUT"), LOG_LVL_FATAL, "inputs.ini:%d missing '='", line_no);
            abort();
        }
        *eq = '\0';
        char* name = trim_ws(p);
        int btn = button_id_from_name(name);
        if (btn < 0) {
#if !DEBUG_BUILD
            if (is_debug_button_name(name)) {
                continue;
            }
#endif
            LOGC(LOGCAT("INPUT"), LOG_LVL_FATAL, "inputs.ini:%d unknown button '%s'", line_no, name);
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
                bind_add(btn, key);
            } else {
                char* endp = NULL;
                key = (int)strtol(key_str, &endp, 10);
                if (!endp || *trim_ws(endp) != '\0') {
                    LOGC(LOGCAT("INPUT"), LOG_LVL_FATAL,
                         "inputs.ini:%d unknown key '%s' for %s", line_no, key_str, name);
                    abort();
                }
                bind_add(btn, key);
            }
            if (!comma) break;
            tok = comma + 1;
        }
    }

    if (ferror(f)) {
        LOGC(LOGCAT("INPUT"), LOG_LVL_FATAL, "inputs.ini: read error");
        abort();
    }
    fclose(f);
    return true;
}

void input_init(void)
{
    s_prev_down_bits   = 0;
    s_edges_available  = false;
    if (!input_load_bindings()) {
        input_init_defaults();
        input_save_bindings();
    }
}

/*
  Public binding API: attach a Raylib key/mouse code to a logical button.
*/
void input_bind(button_t btn, int keycode){
    bind_add(btn, keycode);
}

/*
  Physical polling helpers:
  Raylib uses disjoint ranges for keyboard vs mouse.
  We detect which family the code belongs to and query the proper function.
*/
static bool is_mouse_code(int code){
    return code >= MOUSE_BUTTON_LEFT && code <= MOUSE_BUTTON_BACK;
}

static bool phys_is_down(int code){
    if (is_mouse_code(code)) return IsMouseButtonDown(code);
    if (code >= KEY_NULL && code <= KEY_KB_MENU) return IsKeyDown(code);
    return false;
}

static bool phys_is_pressed(int code){
    if (is_mouse_code(code)) return IsMouseButtonPressed(code);
    if (code >= KEY_NULL && code <= KEY_KB_MENU) return IsKeyPressed(code);
    return false;
}

/*
  Poll the OS once per render frame and compute:
  - 'down' bitset for buttons that are held
  - 'pressed' bitset for rising edges (including Raylib's own edge helpers)
  Also capture mouse position and wheel delta, and derive a normalized move axis.
*/
void input_begin_frame(void){
    input_t in;
    memset(&in, 0, sizeof(in));

    uint64_t down_bits = 0;
    for (int b = 0; b < BTN_COUNT; ++b){
        for (int i = 0; i < g_bindings[b].key_count; ++i){
            if (phys_is_down(g_bindings[b].keys[i])) { down_bits |= BUTTON_BIT(b); break; }
        }
    }

    // edges seen THIS render frame
    uint64_t pressed_now = down_bits & ~s_prev_down_bits;
    for (int b = 0; b < BTN_COUNT; ++b){
        bool any_pressed = false;
        for (int i = 0; i < g_bindings[b].key_count; ++i){
            if (phys_is_pressed(g_bindings[b].keys[i])) { any_pressed = true; break; }
        }
        if (any_pressed) pressed_now |= BUTTON_BIT(b);
    }

    // LATCH edges + wheel until a fixed tick consumes them
    s_latched_pressed |= pressed_now;
    s_latched_wheel   += GetMouseWheelMove();

    s_prev_down_bits = down_bits;

    Vector2 m = GetMousePosition();
    in.down    = down_bits;
    in.pressed = s_latched_pressed;   // <- latched
    in.mouse.x = m.x;
    in.mouse.y = m.y;
    in.mouse_wheel = s_latched_wheel; // <- latched

    in.moveX = ((down_bits & BUTTON_BIT(BTN_RIGHT)) ? 1.f : 0.f)
             - ((down_bits & BUTTON_BIT(BTN_LEFT )) ? 1.f : 0.f);
    in.moveY = ((down_bits & BUTTON_BIT(BTN_DOWN )) ? 1.f : 0.f)
             - ((down_bits & BUTTON_BIT(BTN_UP   )) ? 1.f : 0.f);
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
