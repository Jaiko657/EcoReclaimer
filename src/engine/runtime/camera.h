#pragma once
#include "engine/gfx/gfx.h"
#include "engine/ecs/ecs.h"

typedef struct {
    ecs_entity_t target;
    gfx_vec2 position;
    gfx_vec2 offset;
    gfx_rect bounds;
    float zoom;
    float padding;
    float deadzone_x;
    float deadzone_y;
} camera_config_t;

typedef struct {
    gfx_vec2 center;
    float zoom;
    float padding;
} camera_view_t;

// Camera module requires explicit engine-owned initialization via camera_init().
void camera_init(void);
void camera_shutdown(void);
void camera_set_target(ecs_entity_t target);
camera_config_t camera_get_config(void);
void camera_set_config(const camera_config_t* cfg);
void camera_tick(float dt);
camera_view_t camera_get_view(void);
