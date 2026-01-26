#include "engine/gfx/gfx.h"
#include "engine/core/platform/platform.h"

#include <stdlib.h>

struct gfx_texture {
    uint32_t id;
    int width;
    int height;
};

static int g_screen_w = 0;
static int g_screen_h = 0;

bool gfx_init_renderer(platform_window* window)
{
    g_screen_w = platform_window_width(window);
    g_screen_h = platform_window_height(window);
    return true;
}

void gfx_shutdown(void)
{
}

bool gfx_window_ready(void)
{
    return true;
}

int gfx_screen_width(void)
{
    return g_screen_w;
}

int gfx_screen_height(void)
{
    return g_screen_h;
}

void gfx_begin_frame(void)
{
}

void gfx_end_frame(void)
{
}

void gfx_clear(gfx_color color)
{
    (void)color;
}

void gfx_begin_world(const gfx_camera2d* cam)
{
    (void)cam;
}

void gfx_end_world(void)
{
}

gfx_vec2 gfx_screen_to_world(gfx_vec2 screen, const gfx_camera2d* cam)
{
    (void)cam;
    return screen;
}

gfx_vec2 gfx_world_to_screen(gfx_vec2 world, const gfx_camera2d* cam)
{
    (void)cam;
    return world;
}

void gfx_draw_texture_pro(const gfx_texture* tex, gfx_rect src, gfx_rect dst, gfx_vec2 origin, float rotation, gfx_color tint)
{
    (void)tex;
    (void)src;
    (void)dst;
    (void)origin;
    (void)rotation;
    (void)tint;
}

void gfx_draw_rect(gfx_rect r, gfx_color color)
{
    (void)r;
    (void)color;
}

void gfx_draw_rect_lines(gfx_rect r, gfx_color color)
{
    (void)r;
    (void)color;
}

void gfx_draw_line(gfx_vec2 a, gfx_vec2 b, float thickness, gfx_color color)
{
    (void)a;
    (void)b;
    (void)thickness;
    (void)color;
}

void gfx_draw_text(const char* text, int x, int y, int font_size, gfx_color color)
{
    (void)text;
    (void)x;
    (void)y;
    (void)font_size;
    (void)color;
}

int gfx_measure_text(const char* text, int font_size)
{
    (void)text;
    (void)font_size;
    return 0;
}

void gfx_texture_unload(gfx_texture* tex)
{
    free(tex);
}

bool gfx_texture_size(const gfx_texture* tex, int* out_w, int* out_h)
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (!tex) return false;
    if (out_w) *out_w = tex->width;
    if (out_h) *out_h = tex->height;
    return true;
}

void gfx_texture_debug_info(const gfx_texture* tex, gfx_texture_info* out)
{
    if (!out) return;
    if (!tex) {
        out->id = 0;
        out->width = 0;
        out->height = 0;
        return;
    }
    out->id = tex->id;
    out->width = tex->width;
    out->height = tex->height;
}

gfx_texture* gfx_texture_create_rgba8(int width, int height, const unsigned char* pixels)
{
    (void)pixels;
    if (width <= 0 || height <= 0) return NULL;
    gfx_texture* tex = (gfx_texture*)malloc(sizeof(*tex));
    if (!tex) return NULL;
    tex->id = 0;
    tex->width = width;
    tex->height = height;
    return tex;
}

bool gfx_texture_update_rgba8(gfx_texture* tex, int width, int height, const unsigned char* pixels)
{
    (void)pixels;
    if (!tex || width <= 0 || height <= 0) return false;
    tex->width = width;
    tex->height = height;
    return true;
}
