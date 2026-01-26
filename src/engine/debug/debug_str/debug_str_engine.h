#pragma once

#include "shared/utils/build_config.h"

#if DEBUG_BUILD
void debug_str_engine_register_all(void);
#else
static inline void debug_str_engine_register_all(void) {}
#endif
