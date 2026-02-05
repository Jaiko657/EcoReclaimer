#include "engine/gfx/gfx.h"
#include "engine/core/logger/logger.h"

#include "raylib.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

struct gfx_texture {
    Texture2D tex;
};

static float clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static Color to_rl_color(gfx_color c)
{
    unsigned char r = (unsigned char)(clamp01(c.r) * 255.0f + 0.5f);
    unsigned char g = (unsigned char)(clamp01(c.g) * 255.0f + 0.5f);
    unsigned char b = (unsigned char)(clamp01(c.b) * 255.0f + 0.5f);
    unsigned char a = (unsigned char)(clamp01(c.a) * 255.0f + 0.5f);
    return (Color){ .r = r, .g = g, .b = b, .a = a };
}

static Rectangle to_rl_rect(gfx_rect r)
{
    return (Rectangle){ .x = r.x, .y = r.y, .width = r.w, .height = r.h };
}

static Vector2 to_rl_vec2(gfx_vec2 v)
{
    return (Vector2){ .x = v.x, .y = v.y };
}

static Camera2D to_rl_cam(const gfx_camera2d* cam)
{
    if (!cam) return (Camera2D){ .target = (Vector2){ .x = 0.0f, .y = 0.0f }, .offset = (Vector2){ .x = 0.0f, .y = 0.0f }, .rotation = 0.0f, .zoom = 0.0f };
    return (Camera2D){
        .target = to_rl_vec2(cam->target),
        .offset = to_rl_vec2(cam->offset),
        .rotation = cam->rotation,
        .zoom = cam->zoom
    };
}

bool gfx_init_renderer(platform_window* window)
{
    (void)window;
    if (!IsWindowReady()) {
        LOGC(LOGCAT_REND, LOG_LVL_FATAL, "Renderer: window not ready");
        return false;
    }
#if DEBUG_BUILD
    SetTraceLogLevel(LOG_DEBUG);
#else
    SetTraceLogLevel(LOG_WARNING);
#endif
    return true;
}

void gfx_shutdown(void)
{
}

bool gfx_window_ready(void)
{
    return IsWindowReady();
}

int gfx_screen_width(void)
{
    return GetScreenWidth();
}

int gfx_screen_height(void)
{
    return GetScreenHeight();
}

void gfx_begin_frame(void)
{
    BeginDrawing();
}

void gfx_end_frame(void)
{
    EndDrawing();
}

void gfx_clear(gfx_color color)
{
    ClearBackground(to_rl_color(color));
}

void gfx_begin_world(const gfx_camera2d* cam)
{
    BeginMode2D(to_rl_cam(cam));
}

void gfx_end_world(void)
{
    EndMode2D();
}

gfx_vec2 gfx_screen_to_world(gfx_vec2 screen, const gfx_camera2d* cam)
{
    Vector2 out = GetScreenToWorld2D(to_rl_vec2(screen), to_rl_cam(cam));
    return (gfx_vec2){ .x = out.x, .y = out.y  };
}

gfx_vec2 gfx_world_to_screen(gfx_vec2 world, const gfx_camera2d* cam)
{
    Vector2 out = GetWorldToScreen2D(to_rl_vec2(world), to_rl_cam(cam));
    return (gfx_vec2){ .x = out.x, .y = out.y  };
}

void gfx_draw_texture_pro(const gfx_texture* tex, gfx_rect src, gfx_rect dst, gfx_vec2 origin, float rotation, gfx_color tint)
{
    if (!tex || tex->tex.id == 0) return;
    DrawTexturePro(tex->tex, to_rl_rect(src), to_rl_rect(dst), to_rl_vec2(origin), rotation, to_rl_color(tint));
}

void gfx_draw_rect(gfx_rect r, gfx_color color)
{
    DrawRectangleRec(to_rl_rect(r), to_rl_color(color));
}

void gfx_draw_rect_lines(gfx_rect r, gfx_color color)
{
    int rx = (int)floorf(r.x);
    int ry = (int)floorf(r.y);
    int rw = (int)ceilf(r.w);
    int rh = (int)ceilf(r.h);
    DrawRectangleLines(rx, ry, rw, rh, to_rl_color(color));
}

void gfx_draw_line(gfx_vec2 a, gfx_vec2 b, float thickness, gfx_color color)
{
    DrawLineEx(to_rl_vec2(a), to_rl_vec2(b), thickness, to_rl_color(color));
}

void gfx_draw_text(const char* text, int x, int y, int font_size, gfx_color color)
{
    DrawText(text, x, y, font_size, to_rl_color(color));
}

int gfx_measure_text(const char* text, int font_size)
{
    return MeasureText(text, font_size);
}

void gfx_texture_unload(gfx_texture* tex)
{
    if (!tex) return;
    if (tex->tex.id) UnloadTexture(tex->tex);
    free(tex);
}

bool gfx_texture_size(const gfx_texture* tex, int* out_w, int* out_h)
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (!tex) return false;
    if (out_w) *out_w = tex->tex.width;
    if (out_h) *out_h = tex->tex.height;
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
    out->id = tex->tex.id;
    out->width = tex->tex.width;
    out->height = tex->tex.height;
}

gfx_texture* gfx_texture_create_rgba8(int width, int height, const unsigned char* pixels)
{
    if (width <= 0 || height <= 0 || !pixels) return NULL;
    gfx_texture* tex = (gfx_texture*)malloc(sizeof(*tex));
    if (!tex) return NULL;

    Image img = {
        .data = (void*)pixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    tex->tex = LoadTextureFromImage(img);
    if (tex->tex.id == 0) {
        free(tex);
        return NULL;
    }
    GenTextureMipmaps(&tex->tex);
    return tex;
}

bool gfx_texture_update_rgba8(gfx_texture* tex, int width, int height, const unsigned char* pixels)
{
    if (!tex || !pixels || width <= 0 || height <= 0) return false;

    bool sameSize = (width == tex->tex.width) && (height == tex->tex.height);
    if (sameSize && tex->tex.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
        UpdateTexture(tex->tex, pixels);
        GenTextureMipmaps(&tex->tex);
        return true;
    }

    Image img = {
        .data = (void*)pixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    Texture2D new_tex = LoadTextureFromImage(img);
    if (!new_tex.id) return false;
    UnloadTexture(tex->tex);
    tex->tex = new_tex;
    GenTextureMipmaps(&tex->tex);
    return true;
}
