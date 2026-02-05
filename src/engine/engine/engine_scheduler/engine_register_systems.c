#include "engine/engine/engine_scheduler/engine_register_systems.h"

#include "engine/engine/engine_scheduler/engine_scheduler.h"

// Find wrapper definitions via `SYSTEMS_ADAPT_*(sys_*_adapt`.
// The definitions are in their respective modules.
void sys_physics_adapt(float dt, const input_t* in);
void sys_anim_sprite_adapt(float dt, const input_t* in);
void sys_prox_build_adapt(float dt, const input_t* in);
void sys_billboards_adapt(float dt, const input_t* in);
void sys_effects_tick_begin_adapt(float dt, const input_t* in);

void sys_render_begin_adapt(float dt, const input_t* in);
void sys_render_world_prepare_adapt(float dt, const input_t* in);
void sys_render_world_base_adapt(float dt, const input_t* in);
void sys_render_world_fx_adapt(float dt, const input_t* in);
void sys_render_world_sprites_adapt(float dt, const input_t* in);
void sys_render_world_overlays_adapt(float dt, const input_t* in);
void sys_render_world_end_adapt(float dt, const input_t* in);
void sys_render_ui_adapt(float dt, const input_t* in);
void sys_render_end_adapt(float dt, const input_t* in);

void sys_toast_update_adapt(float dt, const input_t* in);
void sys_camera_tick_adapt(float dt, const input_t* in);
void sys_world_apply_edits_adapt(float dt, const input_t* in);
void sys_asset_collect_adapt(float dt, const input_t* in);
#if DEBUG_BUILD
void sys_debug_binds_adapt(float dt, const input_t* in);
#endif

void engine_register_systems(void)
{
    engine_scheduler_register(PHASE_INPUT, -100, sys_effects_tick_begin_adapt, "effects_tick_begin");

    engine_scheduler_register(PHASE_PHYSICS, 100, sys_physics_adapt, "physics");

    engine_scheduler_register(PHASE_SIM_POST, 100, sys_prox_build_adapt, "proximity_view");
    engine_scheduler_register(PHASE_SIM_POST, 200, sys_billboards_adapt, "billboards");
    engine_scheduler_register(PHASE_SIM_POST, 300, sys_world_apply_edits_adapt, "world_apply_edits");

    engine_scheduler_register(PHASE_PRE_RENDER, 100, sys_toast_update_adapt, "toast_update");
    engine_scheduler_register(PHASE_PRE_RENDER, 200, sys_camera_tick_adapt, "camera_tick");
    engine_scheduler_register(PHASE_PRE_RENDER, 300, sys_anim_sprite_adapt, "sprite_anim");

    engine_scheduler_register(PHASE_RENDER, 100, sys_render_begin_adapt, "render_begin");
    engine_scheduler_register(PHASE_RENDER, 200, sys_render_world_prepare_adapt, "render_world_prepare");
    engine_scheduler_register(PHASE_RENDER, 300, sys_render_world_base_adapt, "render_world_base");
    engine_scheduler_register(PHASE_RENDER, 400, sys_render_world_fx_adapt, "render_world_fx");
    engine_scheduler_register(PHASE_RENDER, 500, sys_render_world_sprites_adapt, "render_world_sprites");
    engine_scheduler_register(PHASE_RENDER, 600, sys_render_world_overlays_adapt, "render_world_overlays");
    engine_scheduler_register(PHASE_RENDER, 700, sys_render_world_end_adapt, "render_world_end");
    engine_scheduler_register(PHASE_RENDER, 800, sys_render_ui_adapt, "render_ui");
    engine_scheduler_register(PHASE_RENDER, 900, sys_render_end_adapt, "render_end");
    engine_scheduler_register(PHASE_RENDER, 1000, sys_asset_collect_adapt, "asset_collect");

#if DEBUG_BUILD
    engine_scheduler_register(PHASE_DEBUG, 100, sys_debug_binds_adapt, "debug_binds");
#endif
}
