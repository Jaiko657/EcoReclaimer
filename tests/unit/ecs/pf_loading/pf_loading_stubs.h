#pragma once

#include <stdbool.h>
#include "engine/ecs/ecs.h"
#include "engine/prefab/prefab.h"
#include "engine/prefab/pf_components_engine.h"
#include "game/prefab/pf_components_game.h"

extern int g_cmp_add_position_calls;
extern int g_cmp_add_size_calls;
extern float g_cmp_add_size_last_hx;
extern float g_cmp_add_size_last_hy;
extern int g_prefab_load_calls;
extern char g_prefab_load_path[256];
extern bool g_prefab_load_result;
extern int g_log_warn_calls;

void pf_loading_stub_reset(void);
void pf_loading_stub_set_player(ecs_entity_t e);
