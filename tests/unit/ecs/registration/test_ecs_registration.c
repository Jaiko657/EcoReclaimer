#include "unity.h"

#include "ecs_registration_stubs.h"
#include "engine/ecs/ecs.h"
#include "engine/engine/engine_scheduler/engine_register_systems.h"
#include "game/ecs/game_register_systems.h"

static void assert_registration(int idx, systems_phase_t phase, int order, const char* name)
{
    TEST_ASSERT_TRUE(idx < g_systems_registration_call_count);
    TEST_ASSERT_EQUAL_INT(phase, g_systems_registration_calls[idx].phase);
    TEST_ASSERT_EQUAL_INT(order, g_systems_registration_calls[idx].order);
    TEST_ASSERT_EQUAL_STRING(name, g_systems_registration_calls[idx].name);
}

void setUp(void)
{
    ecs_registration_stubs_reset();
    ecs_init();
}

void tearDown(void)
{
    ecs_shutdown();
}

void test_systems_registration_orders_and_hooks(void)
{
    engine_scheduler_init();
    engine_register_systems();
    game_register_systems();

    TEST_ASSERT_TRUE(g_systems_init_seq > 0);

    TEST_ASSERT_EQUAL_INT(32, g_systems_registration_call_count);

    assert_registration(0, PHASE_INPUT, -100, "effects_tick_begin");
    assert_registration(1, PHASE_PHYSICS, 100, "physics");
    assert_registration(2, PHASE_SIM_POST, 100, "proximity_view");
    assert_registration(3, PHASE_SIM_POST, 200, "billboards");
    assert_registration(4, PHASE_SIM_POST, 300, "world_apply_edits");
    assert_registration(5, PHASE_PRE_RENDER, 100, "toast_update");
    assert_registration(6, PHASE_PRE_RENDER, 200, "camera_tick");
    assert_registration(7, PHASE_PRE_RENDER, 300, "sprite_anim");
    assert_registration(8, PHASE_RENDER, 100, "render_begin");
    assert_registration(9, PHASE_RENDER, 200, "render_world_prepare");
    assert_registration(10, PHASE_RENDER, 300, "render_world_base");
    assert_registration(11, PHASE_RENDER, 400, "render_world_fx");
    assert_registration(12, PHASE_RENDER, 500, "render_world_sprites");
    assert_registration(13, PHASE_RENDER, 600, "render_world_overlays");
    assert_registration(14, PHASE_RENDER, 700, "render_world_end");
    assert_registration(15, PHASE_RENDER, 800, "render_ui");
    assert_registration(16, PHASE_RENDER, 900, "render_end");
    assert_registration(17, PHASE_RENDER, 1000, "asset_collect");
    assert_registration(18, PHASE_INPUT, -95, "input");
    assert_registration(19, PHASE_INPUT, -90, "grav_gun_input");
    assert_registration(20, PHASE_SIM_PRE, 100, "animation_controller");
    assert_registration(21, PHASE_PHYSICS, 90, "grav_gun_motion");
    assert_registration(22, PHASE_PHYSICS, 95, "conveyor_apply");
    assert_registration(23, PHASE_SIM_POST, 105, "conveyor_update");
    assert_registration(24, PHASE_SIM_POST, 110, "recycle_bins");
    assert_registration(25, PHASE_SIM_POST, 115, "recycle_anim");
    assert_registration(26, PHASE_SIM_POST, 120, "storage_deposit");
    assert_registration(27, PHASE_SIM_POST, 130, "unloader_tick");
    assert_registration(28, PHASE_SIM_POST, 150, "grav_gun_tool");
    assert_registration(29, PHASE_SIM_POST, 175, "grav_gun_charger");
    assert_registration(30, PHASE_SIM_POST, 250, "grav_gun_fx");
    assert_registration(31, PHASE_SIM_POST, 295, "doors_tick");
}
