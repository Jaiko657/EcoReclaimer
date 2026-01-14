#include "engine/world/world_door.h"
#include "engine/world/world_door_handle.h"

world_door_handle_t world_door_register(const door_tile_xy_t* tile_xy, size_t tile_count)
{
    (void)tile_xy;
    (void)tile_count;
    return WORLD_DOOR_INVALID_HANDLE;
}

void world_door_unregister(world_door_handle_t handle)
{
    (void)handle;
}

int world_door_primary_animation_duration(world_door_handle_t handle)
{
    (void)handle;
    return 0;
}

void world_door_apply_state(world_door_handle_t handle, float t_ms, bool play_forward)
{
    (void)handle;
    (void)t_ms;
    (void)play_forward;
}

void world_door_shutdown(void)
{
}
