#pragma once

#include <stdbool.h>
#include "engine/gfx/gfx_types.h"

#define GFX_COLOR(r, g, b, a) ((gfx_color){ (float)(r) / 255.0f, (float)(g) / 255.0f, (float)(b) / 255.0f, (float)(a) / 255.0f  })
#define GFX_WHITE  GFX_COLOR(255, 255, 255, 255)
#define GFX_BLACK  GFX_COLOR(0, 0, 0, 255)
#define GFX_RED    GFX_COLOR(255, 0, 0, 255)
#define GFX_GREEN  GFX_COLOR(0, 255, 0, 255)
#define GFX_BLUE   GFX_COLOR(0, 0, 255, 255)
#define GFX_GRAY   GFX_COLOR(130, 130, 130, 255)
#define GFX_RAYWHITE GFX_COLOR(245, 245, 245, 255)
#define GFX_LIGHTGRAY GFX_COLOR(200, 200, 200, 255)
#define GFX_DARKGRAY  GFX_COLOR(80, 80, 80, 255)
#define GFX_BROWN     GFX_COLOR(127, 106, 79, 255)

struct platform_window;
typedef struct platform_window platform_window;

bool gfx_init_renderer(platform_window* window);
void gfx_shutdown(void);
bool gfx_window_ready(void);

int gfx_screen_width(void);
int gfx_screen_height(void);

void gfx_begin_frame(void);
void gfx_end_frame(void);
void gfx_clear(gfx_color color);

void gfx_begin_world(const gfx_camera2d* cam);
void gfx_end_world(void);

gfx_vec2 gfx_screen_to_world(gfx_vec2 screen, const gfx_camera2d* cam);
gfx_vec2 gfx_world_to_screen(gfx_vec2 world, const gfx_camera2d* cam);

void gfx_draw_texture_pro(const gfx_texture* tex, gfx_rect src, gfx_rect dst, gfx_vec2 origin, float rotation, gfx_color tint);
void gfx_draw_rect(gfx_rect r, gfx_color color);
void gfx_draw_rect_lines(gfx_rect r, gfx_color color);
void gfx_draw_line(gfx_vec2 a, gfx_vec2 b, float thickness, gfx_color color);

void gfx_draw_text(const char* text, int x, int y, int font_size, gfx_color color);
int gfx_measure_text(const char* text, int font_size);

void gfx_texture_unload(gfx_texture* tex);
bool gfx_texture_size(const gfx_texture* tex, int* out_w, int* out_h);
void gfx_texture_debug_info(const gfx_texture* tex, gfx_texture_info* out);

// Create/update textures from RGBA8 pixels (pixels are CPU-owned by caller).
gfx_texture* gfx_texture_create_rgba8(int width, int height, const unsigned char* pixels);
bool gfx_texture_update_rgba8(gfx_texture* tex, int width, int height, const unsigned char* pixels);
