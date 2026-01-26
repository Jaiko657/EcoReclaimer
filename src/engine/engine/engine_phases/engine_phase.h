#pragma once

typedef enum {
    ENGINE_PHASE_GAME_INIT = 0,
    ENGINE_PHASE_POST_ENTITIES,
    ENGINE_PHASE_PRE_SHUTDOWN,
    ENGINE_PHASE_COUNT
} engine_phase_t;

typedef void (*engine_phase_fn)(engine_phase_t phase, void* data);

void engine_phase_init(void);
void engine_phase_shutdown(void);
void engine_phase_register(engine_phase_t phase, int order, engine_phase_fn fn, void* data, const char* name);
void engine_phase_run(engine_phase_t phase);
