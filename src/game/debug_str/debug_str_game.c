#include "game/debug_str/debug_str_game.h"

#if DEBUG_BUILD

void debug_str_register_player(void);
void debug_str_register_resource(void);
void debug_str_register_storage(void);
void debug_str_register_recycle_bin(void);
void debug_str_register_liftable(void);
void debug_str_register_conveyor(void);
void debug_str_register_conveyor_rider(void);
void debug_str_register_door(void);
void debug_str_register_grav_gun(void);
void debug_str_register_gun_charger(void);
void debug_str_register_unpacker(void);
void debug_str_register_unloader(void);

void debug_str_game_register_all(void)
{
    debug_str_register_player();
    debug_str_register_resource();
    debug_str_register_storage();
    debug_str_register_recycle_bin();
    debug_str_register_liftable();
    debug_str_register_conveyor();
    debug_str_register_conveyor_rider();
    debug_str_register_door();
    debug_str_register_grav_gun();
    debug_str_register_gun_charger();
    debug_str_register_unpacker();
    debug_str_register_unloader();
}

#endif
