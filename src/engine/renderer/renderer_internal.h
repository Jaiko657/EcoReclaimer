#pragma once

#include "engine/gfx/gfx.h"
#include "engine/ecs/ecs.h"
#include "engine/ecs/ecs_render.h"
#include "engine/utils/dynarray.h"
#include "engine/tiled/tiled.h"

typedef struct {
    ecs_sprite_view_t v;
    float key;
    int   seq; // insertion order, tie breaker
} Item;

typedef DA(Item) ItemArray;

typedef struct {
    ItemArray* queue;
    int dropped;
} painter_queue_ctx_t;

typedef struct render_view_t {
    gfx_camera2d cam;
    gfx_rect view;
    gfx_rect padded_view;
} render_view_t;

typedef struct {
    const world_map_t* map;
    bool has_vis;
    int startX;
    int startY;
    int endX;
    int endY;
    int visible_tiles;
    int painter_cap;
    double now_ms;
} render_world_cache_t;

typedef struct {
    tiled_renderer_t tiled;
    uint32_t bound_gen;
    ItemArray painter_items;
    render_view_t frame_view;
    render_world_cache_t world_cache;
    painter_queue_ctx_t painter_ctx;
    bool frame_active;
    bool painter_ready;
} renderer_ctx_t;

renderer_ctx_t* renderer_ctx_get(void);

// ===== renderer frame pipeline =====
render_view_t build_camera_view(void);
void renderer_frame_begin(void);
void renderer_frame_end(void);
void renderer_world_prepare(const render_view_t* view);
void renderer_world_base(const render_view_t* view);
void renderer_world_fx(const render_view_t* view);
void renderer_world_sprites(const render_view_t* view);
void renderer_world_overlays(const render_view_t* view);
void renderer_world_end(void);
void renderer_ui(const render_view_t* view);
void draw_screen_space_ui(const render_view_t* view);
void draw_effect_lines(const render_view_t* view);
bool visible_tile_range(const world_map_t* map,
                        gfx_rect padded_view,
                        int* out_startX, int* out_startY,
                        int* out_endX, int* out_endY,
                        int* out_visible_tiles);
void draw_tmx_stack(const world_map_t* map,
                    const render_view_t* view,
                    int startX, int startY, int endX, int endY,
                    double now_ms,
                    painter_queue_ctx_t* painter_ctx);
void draw_world_fallback_tiles(const render_view_t* view);
void enqueue_ecs_sprites(const render_view_t* view, painter_queue_ctx_t* painter_ctx);
void flush_painter_queue(painter_queue_ctx_t* painter_ctx);
void renderer_painter_prepare(renderer_ctx_t* ctx, int max_items);
void renderer_painter_ensure_ready(renderer_ctx_t* ctx);
unsigned char u8(float x);
gfx_rect expand_rect(gfx_rect r, float margin);
gfx_rect intersect_rect(gfx_rect a, gfx_rect b);
bool rects_intersect(gfx_rect a, gfx_rect b);
gfx_rect sprite_bounds(const ecs_sprite_view_t* v);
bool painter_queue_push(painter_queue_ctx_t* ctx, Item item);
void draw_debug_collision_overlays(const render_view_t* view);
void draw_debug_trigger_overlays(const render_view_t* view);
void renderer_debug_draw_ui(const render_view_t* view);
