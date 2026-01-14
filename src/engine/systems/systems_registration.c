#include "engine/systems/systems.h"
#include "engine/systems/systems_registration.h"

// Find wrapper definitions via `SYSTEMS_ADAPT_*(sys_*_adapt`.
// The definitions are in their respective modules.
void sys_input_adapt(float dt, const input_t* in);
void sys_physics_adapt(float dt, const input_t* in);
void sys_anim_controller_adapt(float dt, const input_t* in);
void sys_anim_sprite_adapt(float dt, const input_t* in);
void sys_prox_build_adapt(float dt, const input_t* in);
void sys_billboards_adapt(float dt, const input_t* in);
void sys_grav_gun_input_adapt(float dt, const input_t* in);
void sys_grav_gun_tool_adapt(float dt, const input_t* in);
void sys_grav_gun_motion_adapt(float dt, const input_t* in);
void sys_grav_gun_charger_adapt(float dt, const input_t* in);
void sys_effects_tick_begin_adapt(float dt, const input_t* in);
void sys_grav_gun_fx_adapt(float dt, const input_t* in);
void sys_conveyor_update_adapt(float dt, const input_t* in);
void sys_conveyor_apply_adapt(float dt, const input_t* in);
void sys_render_begin_adapt(float dt, const input_t* in);
void sys_render_world_prepare_adapt(float dt, const input_t* in);
void sys_render_world_base_adapt(float dt, const input_t* in);
void sys_render_world_fx_adapt(float dt, const input_t* in);
void sys_render_world_sprites_adapt(float dt, const input_t* in);
void sys_render_world_overlays_adapt(float dt, const input_t* in);
void sys_render_world_end_adapt(float dt, const input_t* in);
void sys_render_ui_adapt(float dt, const input_t* in);
void sys_render_end_adapt(float dt, const input_t* in);

void sys_recycle_bins_adapt(float dt, const input_t* in);
void sys_recycle_anim_adapt(float dt, const input_t* in);
void sys_storage_deposit_adapt(float dt, const input_t* in);
void sys_unloader_tick_adapt(float dt, const input_t* in);
void sys_doors_tick_adapt(float dt, const input_t* in);

void sys_toast_update_adapt(float dt, const input_t* in);
void sys_camera_tick_adapt(float dt, const input_t* in);
void sys_world_apply_edits_adapt(float dt, const input_t* in);
void sys_asset_collect_adapt(float dt, const input_t* in);
#if DEBUG_BUILD
void sys_debug_binds_adapt(float dt, const input_t* in);
#endif

void systems_registration_init(void)
{
    systems_init();

    // Pipeline mapping: Input -> Intent -> Physics -> Post-Sim -> Present -> Render -> GC.
    systems_register(PHASE_INPUT, -100, sys_effects_tick_begin_adapt, "effects_tick_begin");
    systems_register(PHASE_INPUT, 0, sys_input_adapt, "input");
    systems_register(PHASE_INPUT, 50, sys_grav_gun_input_adapt, "grav_gun_input");

    systems_register(PHASE_SIM_PRE, 100, sys_anim_controller_adapt, "animation_controller");

    systems_register(PHASE_PHYSICS, 90, sys_grav_gun_motion_adapt, "grav_gun_motion");
    systems_register(PHASE_PHYSICS, 95, sys_conveyor_apply_adapt, "conveyor_apply");
    systems_register(PHASE_PHYSICS, 100, sys_physics_adapt, "physics");

    systems_register(PHASE_SIM_POST, 100, sys_prox_build_adapt, "proximity_view");
    systems_register(PHASE_SIM_POST, 105, sys_conveyor_update_adapt, "conveyor_update");
    systems_register(PHASE_SIM_POST, 110, sys_recycle_bins_adapt, "recycle_bins");
    systems_register(PHASE_SIM_POST, 115, sys_recycle_anim_adapt, "recycle_anim");
    systems_register(PHASE_SIM_POST, 120, sys_storage_deposit_adapt, "storage_deposit");
    systems_register(PHASE_SIM_POST, 130, sys_unloader_tick_adapt, "unloader_tick");
    systems_register(PHASE_SIM_POST, 150, sys_grav_gun_tool_adapt, "grav_gun_tool");
    systems_register(PHASE_SIM_POST, 175, sys_grav_gun_charger_adapt, "grav_gun_charger");
    systems_register(PHASE_SIM_POST, 200, sys_billboards_adapt, "billboards");
    systems_register(PHASE_SIM_POST, 250, sys_grav_gun_fx_adapt, "grav_gun_fx");
    systems_register(PHASE_SIM_POST, 400, sys_doors_tick_adapt, "doors_tick");
    systems_register(PHASE_SIM_POST, 900, sys_world_apply_edits_adapt, "world_apply_edits");

    systems_register(PHASE_PRE_RENDER, 10, sys_toast_update_adapt, "toast_update");
    systems_register(PHASE_PRE_RENDER, 20, sys_camera_tick_adapt, "camera_tick");
    systems_register(PHASE_PRE_RENDER, 100, sys_anim_sprite_adapt, "sprite_anim");

    systems_register(PHASE_RENDER, 10, sys_render_begin_adapt, "render_begin");
    systems_register(PHASE_RENDER, 20, sys_render_world_prepare_adapt, "render_world_prepare");
    systems_register(PHASE_RENDER, 30, sys_render_world_base_adapt, "render_world_base");
    systems_register(PHASE_RENDER, 40, sys_render_world_fx_adapt, "render_world_fx");
    systems_register(PHASE_RENDER, 50, sys_render_world_sprites_adapt, "render_world_sprites");
    systems_register(PHASE_RENDER, 60, sys_render_world_overlays_adapt, "render_world_overlays");
    systems_register(PHASE_RENDER, 70, sys_render_world_end_adapt, "render_world_end");
    systems_register(PHASE_RENDER, 80, sys_render_ui_adapt, "render_ui");
    systems_register(PHASE_RENDER, 90, sys_render_end_adapt, "render_end");
    systems_register(PHASE_RENDER, 1000, sys_asset_collect_adapt, "asset_collect");

#if DEBUG_BUILD
    systems_register(PHASE_DEBUG, 100, sys_debug_binds_adapt, "debug_binds");
#endif

}
