#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "engine/gfx/gfx_types.h"

typedef struct {
    uint32_t id;
    int width;
    int height;
} AssetBackendDebugInfo;

gfx_texture* asset_backend_load_texture(const char* path);
void asset_backend_unload_texture(gfx_texture* tex);
bool asset_backend_texture_size(const gfx_texture* tex, int* out_w, int* out_h);
void asset_backend_debug_info(const gfx_texture* tex, AssetBackendDebugInfo* out);

void asset_backend_reload_all_begin(void);
void asset_backend_reload_texture(gfx_texture* tex, const char* path);
void asset_backend_reload_all_end(void);
