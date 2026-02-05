#pragma once


#if DEBUG_BUILD
void debug_post_frame(void);
#else
static inline void debug_post_frame(void) { }
#endif
