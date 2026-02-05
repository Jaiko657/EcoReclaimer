#pragma once
#include <stdbool.h>
#include "engine/ecs/ecs.h"

typedef struct {
    int _reserved;
} engine_game_hooks_t;

bool engine_init(const char *title);
// returns process exit code (0 OK, non-zero error)
int engine_run(void);
void engine_shutdown(void);
bool engine_reload_world(void);
bool engine_reload_world_from_path(const char* tmx_path);
bool engine_set_world_tmx_path(const char* tmx_path);
