#pragma once

#include <stdint.h>

typedef struct { float x, y; } gfx_vec2;
typedef struct { float x, y, w, h; } gfx_rect;
// Colors are normalized floats in 0..1.
typedef struct { float r, g, b, a; } gfx_color;

typedef struct gfx_texture gfx_texture;

typedef struct {
    gfx_vec2 target;
    gfx_vec2 offset;
    float rotation;
    float zoom;
} gfx_camera2d;

typedef struct {
    uint32_t id;
    int width;
    int height;
} gfx_texture_info;

static inline gfx_vec2 gfx_vec2_make(float x, float y) { return (gfx_vec2){ .x = x, .y = y  }; }
static inline gfx_rect gfx_rect_xywh(float x, float y, float w, float h) { return (gfx_rect){ .x = x, .y = y, .w = w, .h = h  }; }
