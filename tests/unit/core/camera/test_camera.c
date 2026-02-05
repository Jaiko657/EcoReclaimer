#include "unity.h"

#include "engine/runtime/camera.h"
#include "ecs_stubs.h"

void setUp(void)
{
    camera_init();
    ecs_stub_reset();
}

void tearDown(void)
{
}

void test_camera_deadzone_moves_toward_target(void)
{
    ecs_entity_t target = (ecs_entity_t){ .idx = 1, .gen = 1 };
    ecs_stub_set_target(target);
    ecs_stub_set_position(true, gfx_vec2_make(100.0f, 0.0f));

    camera_config_t cfg = camera_get_config();
    cfg.target = target;
    cfg.position = gfx_vec2_make(0.0f, 0.0f);
    cfg.offset = gfx_vec2_make(0.0f, 0.0f);
    cfg.deadzone_x = 10.0f;
    cfg.deadzone_y = 10.0f;
    cfg.bounds = gfx_rect_xywh(0.0f, 0.0f, 0.0f, 0.0f); // no clamp
    camera_set_config(&cfg);

    camera_tick(0.0f);
    camera_view_t v = camera_get_view();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 90.0f, v.center.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, v.center.y);
}

void test_camera_bounds_clamps_center(void)
{
    ecs_entity_t target = (ecs_entity_t){ .idx = 1, .gen = 1 };
    ecs_stub_set_target(target);
    ecs_stub_set_position(true, gfx_vec2_make(100.0f, 100.0f));

    camera_config_t cfg = camera_get_config();
    cfg.target = target;
    cfg.position = gfx_vec2_make(0.0f, 0.0f);
    cfg.deadzone_x = 0.0f;
    cfg.deadzone_y = 0.0f;
    cfg.bounds = gfx_rect_xywh(0.0f, 0.0f, 50.0f, 25.0f);
    camera_set_config(&cfg);

    camera_tick(0.0f);
    camera_view_t v = camera_get_view();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 50.0f, v.center.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 25.0f, v.center.y);
}

void test_camera_set_config_clamps_invalid_values(void)
{
    camera_config_t cfg = camera_get_config();
    cfg.zoom = 0.0f;
    cfg.padding = -1.0f;
    cfg.deadzone_x = -2.0f;
    cfg.deadzone_y = -3.0f;
    camera_set_config(&cfg);

    camera_view_t v = camera_get_view();
    TEST_ASSERT_TRUE(v.zoom > 0.0f);
    TEST_ASSERT_TRUE(v.padding >= 0.0f);
}

void test_camera_set_target_before_init_is_not_buffered(void)
{
    ecs_entity_t target = (ecs_entity_t){ .idx = 1, .gen = 1 };
    ecs_stub_set_target(target);
    ecs_stub_set_position(true, gfx_vec2_make(100.0f, 0.0f));

    camera_shutdown();
    camera_set_target(target);
    camera_init();

    camera_tick(0.0f);
    camera_view_t v = camera_get_view();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, v.center.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, v.center.y);
}

void test_camera_set_config_before_init_is_ignored(void)
{
    camera_shutdown();
    camera_config_t cfg = {
        .target = (ecs_entity_t){ .idx = 1, .gen = 1 },
        .position = gfx_vec2_make(50.0f, 60.0f),
        .offset = gfx_vec2_make(1.0f, 2.0f),
        .bounds = gfx_rect_xywh(5.0f, 6.0f, 10.0f, 12.0f),
        .zoom = 2.0f,
        .padding = 10.0f,
        .deadzone_x = 3.0f,
        .deadzone_y = 4.0f,
    };
    camera_set_config(&cfg);
    camera_init();

    camera_view_t v = camera_get_view();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, v.center.x);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, v.center.y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, v.zoom);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 64.0f, v.padding);
}
