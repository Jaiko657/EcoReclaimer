#pragma once
#include <stdbool.h>
#include "engine/ecs/ecs.h"

typedef struct render_view_t render_view_t;

typedef struct {
    void (*game_init)(void);
    void (*ecs_game_shutdown)(void);
    ecs_entity_t (*ecs_find_player)(void);
    void (*render_game_ui)(const render_view_t* view);
} engine_game_hooks_t;

void engine_register_game_hooks(engine_game_hooks_t hooks);
const engine_game_hooks_t* engine_get_game_hooks(void);

bool engine_init(const char *title);
// returns process exit code (0 OK, non-zero error)
int engine_run(void);
void engine_shutdown(void);
bool engine_reload_world(void);
bool engine_reload_world_from_path(const char* tmx_path);
