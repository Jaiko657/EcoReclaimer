#pragma once


#if DEBUG_BUILD
void debug_str_game_register_all(void);
#else
static inline void debug_str_game_register_all(void) {}
#endif
