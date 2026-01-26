#include "engine/renderer/renderer_internal.h"
#include "engine/asset/asset_renderer_internal.h"
#include "engine/core/logger/logger.h"
#include "engine/core/time/time.h"

#include <math.h>
#include <stdlib.h>

static float clampf_local(float v, float lo, float hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

static void draw_sprite_highlight(const gfx_texture* tex, gfx_rect src, gfx_rect dst, gfx_vec2 origin, gfx_color color)
{
    if (color.a <= 0.0f) return;

    float t = (sinf((float)time_now() * 6.0f) + 1.0f) * 0.5f;
    float base_a = clampf_local(color.a, 0.0f, 1.0f);
    float pulse = (60.0f + t * 120.0f) * base_a;
    gfx_color tint = (gfx_color){ .r = color.r, .g = color.g, .b = color.b, .a = pulse / 255.0f  };

    gfx_draw_texture_pro(tex, src, dst, origin, 0.0f, tint);
}

void renderer_painter_prepare(renderer_ctx_t* ctx, int max_items)
{
    if (!ctx) return;
    if (max_items < 0) max_items = 0;
    DA_RESERVE(&ctx->painter_items, (size_t)max_items);
    DA_CLEAR(&ctx->painter_items);
    ctx->painter_ctx = (painter_queue_ctx_t){ .queue = &ctx->painter_items, .dropped = 0 };
    ctx->painter_ready = true;
}

void renderer_painter_ensure_ready(renderer_ctx_t* ctx)
{
    if (!ctx) return;
    if (!ctx->painter_ready) {
        DA_CLEAR(&ctx->painter_items);
        ctx->painter_ctx = (painter_queue_ctx_t){ .queue = &ctx->painter_items, .dropped = 0 };
        ctx->painter_ready = true;
    }
}

bool painter_queue_push(painter_queue_ctx_t* ctx, Item item)
{
    if (!ctx || !ctx->queue) return false;
    if (ctx->queue->size >= ctx->queue->capacity) {
        DA_GROW(ctx->queue);
    }
    if (ctx->queue->size >= ctx->queue->capacity) {
        ctx->dropped++;
        return false;
    }
    ctx->queue->data[ctx->queue->size++] = item;
    return true;
}

static int cmp_item(const void* a, const void* b)
{
    const Item* A = (const Item*)a;
    const Item* B = (const Item*)b;
    if (A->key < B->key) return -1;
    if (A->key > B->key) return  1;
    if (A->seq < B->seq) return -1;
    if (A->seq > B->seq) return  1;
    return 0;
}

void flush_painter_queue(painter_queue_ctx_t* painter_ctx)
{
    if (!painter_ctx || !painter_ctx->queue) return;
    if (painter_ctx->dropped > 0) {
        LOGC(LOGCAT_REND, LOG_LVL_WARN, "painter queue overflow; dropped %d items", painter_ctx->dropped);
    }

    qsort(painter_ctx->queue->data, painter_ctx->queue->size, sizeof(Item), cmp_item);

    for (size_t i = 0; i < painter_ctx->queue->size; ++i) {
        ecs_sprite_view_t v = painter_ctx->queue->data[i].v;

        const gfx_texture* t = asset_lookup_texture(v.tex);
        if (!t) continue;

        gfx_rect src = (gfx_rect){ .x = v.src.x, .y = v.src.y, .w = v.src.w, .h = v.src.h  };
        gfx_rect dst = (gfx_rect){ .x = v.x, .y = v.y, .w = fabsf(v.src.w), .h = fabsf(v.src.h)  };
        gfx_vec2 origin = (gfx_vec2){ .x = v.ox, .y = v.oy  };

        gfx_draw_texture_pro(t, src, dst, origin, 0.0f, GFX_WHITE);
        if (v.highlighted && v.highlight_color.a > 0.0f) {
            draw_sprite_highlight(t, src, dst, origin, v.highlight_color);
        }
    }
}
