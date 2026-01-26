#include "engine/asset/asset_backend.h"
#include "engine/core/logger/logger.h"
#include "engine/gfx/gfx.h"

#if defined(HEADLESS)
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image.h"

#include <stdbool.h>
#include <stdlib.h>

static unsigned char* decode_image_rgba8(const char* path, int* out_w, int* out_h)
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (!path) return NULL;

    int w = 0;
    int h = 0;
    int comp = 0;
    unsigned char* pixels = stbi_load(path, &w, &h, &comp, 4);
    if (!pixels) {
        LOGC(LOGCAT_ASSET, LOG_LVL_ERROR, "asset: failed to decode '%s'", path);
        return NULL;
    }
    if (out_w) *out_w = w;
    if (out_h) *out_h = h;
    return pixels;
}

gfx_texture* asset_backend_load_texture(const char* path)
{
    int w = 0;
    int h = 0;
    unsigned char* pixels = decode_image_rgba8(path, &w, &h);
    if (!pixels) return NULL;

    gfx_texture* tex = gfx_texture_create_rgba8(w, h, pixels);
    stbi_image_free(pixels);
    return tex;
}

void asset_backend_unload_texture(gfx_texture* tex)
{
    if (!tex) return;
    gfx_texture_unload(tex);
}

bool asset_backend_texture_size(const gfx_texture* tex, int* out_w, int* out_h)
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (!tex) {
        LOGC(LOGCAT_ASSET, LOG_LVL_WARN, "asset_backend_texture_size: null texture");
        return false;
    }
    return gfx_texture_size(tex, out_w, out_h);
}

void asset_backend_debug_info(const gfx_texture* tex, AssetBackendDebugInfo* out)
{
    if (!out) return;
    if (!tex) {
        out->id = 0;
        out->width = 0;
        out->height = 0;
        return;
    }
    gfx_texture_info info = {0};
    gfx_texture_debug_info(tex, &info);
    out->id = info.id;
    out->width = info.width;
    out->height = info.height;
}

void asset_backend_reload_all_begin(void)
{
    LOGC(LOGCAT_ASSET, LOG_LVL_INFO, "Reloading all textures...");
}

void asset_backend_reload_texture(gfx_texture* tex, const char* path)
{
    if (!tex || !path) return;
    int w = 0;
    int h = 0;
    unsigned char* pixels = decode_image_rgba8(path, &w, &h);
    if (!pixels) return;
    (void)gfx_texture_update_rgba8(tex, w, h, pixels);
    stbi_image_free(pixels);
}

void asset_backend_reload_all_end(void)
{
    LOGC(LOGCAT_ASSET, LOG_LVL_INFO, "Reload complete.");
}
