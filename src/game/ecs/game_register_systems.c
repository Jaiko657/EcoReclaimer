#include "game/ecs/game_register_systems.h"

#include "engine/engine/engine_scheduler/engine_scheduler.h"

// Find wrapper definitions via `SYSTEMS_ADAPT_*(sys_*_adapt`.
// The definitions are in their respective modules.
void sys_input_adapt(float dt, const input_t* in);
void sys_anim_controller_adapt(float dt, const input_t* in);
void sys_grav_gun_input_adapt(float dt, const input_t* in);
void sys_grav_gun_tool_adapt(float dt, const input_t* in);
void sys_grav_gun_motion_adapt(float dt, const input_t* in);
void sys_grav_gun_charger_adapt(float dt, const input_t* in);
void sys_grav_gun_fx_adapt(float dt, const input_t* in);
void sys_conveyor_update_adapt(float dt, const input_t* in);
void sys_conveyor_apply_adapt(float dt, const input_t* in);
void sys_recycle_bins_adapt(float dt, const input_t* in);
void sys_recycle_anim_adapt(float dt, const input_t* in);
void sys_storage_deposit_adapt(float dt, const input_t* in);
void sys_unloader_tick_adapt(float dt, const input_t* in);
void sys_doors_tick_adapt(float dt, const input_t* in);

void game_register_systems(void)
{
    engine_scheduler_register(PHASE_INPUT, -95, sys_input_adapt, "input");
    engine_scheduler_register(PHASE_INPUT, -90, sys_grav_gun_input_adapt, "grav_gun_input");

    engine_scheduler_register(PHASE_SIM_PRE, 100, sys_anim_controller_adapt, "animation_controller");

    engine_scheduler_register(PHASE_PHYSICS, 90, sys_grav_gun_motion_adapt, "grav_gun_motion");
    engine_scheduler_register(PHASE_PHYSICS, 95, sys_conveyor_apply_adapt, "conveyor_apply");

    engine_scheduler_register(PHASE_SIM_POST, 105, sys_conveyor_update_adapt, "conveyor_update");
    engine_scheduler_register(PHASE_SIM_POST, 110, sys_recycle_bins_adapt, "recycle_bins");
    engine_scheduler_register(PHASE_SIM_POST, 115, sys_recycle_anim_adapt, "recycle_anim");
    engine_scheduler_register(PHASE_SIM_POST, 120, sys_storage_deposit_adapt, "storage_deposit");
    engine_scheduler_register(PHASE_SIM_POST, 130, sys_unloader_tick_adapt, "unloader_tick");
    engine_scheduler_register(PHASE_SIM_POST, 150, sys_grav_gun_tool_adapt, "grav_gun_tool");
    engine_scheduler_register(PHASE_SIM_POST, 175, sys_grav_gun_charger_adapt, "grav_gun_charger");
    engine_scheduler_register(PHASE_SIM_POST, 250, sys_grav_gun_fx_adapt, "grav_gun_fx");
    engine_scheduler_register(PHASE_SIM_POST, 295, sys_doors_tick_adapt, "doors_tick");
}
