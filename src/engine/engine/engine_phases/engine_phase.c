#include "engine/engine/engine_phases/engine_phase.h"
#include "engine/core/logger/logger.h"
#include "shared/utils/build_config.h"
#include "shared/utils/dynarray.h"

typedef struct {
    engine_phase_fn fn;
    void* data;
    const char* name;
    int order;
} engine_phase_entry_t;

static DA(engine_phase_entry_t) g_phase_registry[ENGINE_PHASE_COUNT];
static bool g_phase_initialized = false;

#if ENGINE_PHASE_LOGGING
static const char* engine_phase_name(engine_phase_t phase)
{
    switch (phase) {
    case ENGINE_PHASE_GAME_INIT: return "ENGINE_PHASE_GAME_INIT";
    case ENGINE_PHASE_POST_ENTITIES: return "ENGINE_PHASE_POST_ENTITIES";
    case ENGINE_PHASE_PRE_SHUTDOWN: return "ENGINE_PHASE_PRE_SHUTDOWN";
    default: return "ENGINE_PHASE_UNKNOWN";
    }
}
#endif

static inline void engine_phase_log_debug(const char* fmt, engine_phase_t phase, int order, const char* name, size_t count)
{
#if ENGINE_PHASE_LOGGING
    LOGC(LOGCAT(PHASE), LOG_LVL_DEBUG, fmt, engine_phase_name(phase), order, name ? name : "(unnamed)", count);
#else
    (void)fmt;
    (void)phase;
    (void)order;
    (void)name;
    (void)count;
#endif
}

static inline void engine_phase_log_warn(const char* fmt, engine_phase_t phase, const char* name)
{
#if ENGINE_PHASE_LOGGING
    LOGC(LOGCAT(PHASE), LOG_LVL_WARN, fmt, phase, name ? name : "(unnamed)");
#else
    (void)fmt;
    (void)phase;
    (void)name;
#endif
}

static void sort_phase(engine_phase_t phase)
{
    size_t n = g_phase_registry[phase].size;
    for (size_t i = 1; i < n; ++i) {
        engine_phase_entry_t key = g_phase_registry[phase].data[i];
        size_t j = i;
        while (j > 0 && g_phase_registry[phase].data[j - 1].order > key.order) {
            g_phase_registry[phase].data[j] = g_phase_registry[phase].data[j - 1];
            --j;
        }
        g_phase_registry[phase].data[j] = key;
    }
}

void engine_phase_init(void)
{
    if (g_phase_initialized) return;
    for (int i = 0; i < ENGINE_PHASE_COUNT; ++i) {
        DA_CLEAR(&g_phase_registry[i]);
    }
    g_phase_initialized = true;
}

void engine_phase_shutdown(void)
{
    for (int i = 0; i < ENGINE_PHASE_COUNT; ++i) {
        DA_FREE(&g_phase_registry[i]);
    }
    g_phase_initialized = false;
}

void engine_phase_register(engine_phase_t phase, int order, engine_phase_fn fn, void* data, const char* name)
{
    if (!fn) return;
    if (phase < 0 || phase >= ENGINE_PHASE_COUNT) {
        engine_phase_log_warn("engine_phase_register: invalid phase %d for %s", phase, name);
        return;
    }
    engine_phase_entry_t entry = {
        .fn = fn,
        .data = data,
        .name = name,
        .order = order
    };
    DA_APPEND(&g_phase_registry[phase], entry);
    sort_phase(phase);
    engine_phase_log_debug("engine_phase_register: %s order=%d name=%s count=%zu", phase, order, name, g_phase_registry[phase].size);
}

void engine_phase_run(engine_phase_t phase)
{
    if (phase < 0 || phase >= ENGINE_PHASE_COUNT) return;
    engine_phase_log_debug("engine_phase_run: %s order=%d name=%s count=%zu", phase, 0, NULL, g_phase_registry[phase].size);
    for (size_t i = 0; i < g_phase_registry[phase].size; ++i) {
        engine_phase_entry_t entry = g_phase_registry[phase].data[i];
        engine_phase_log_debug("engine_phase_run: %s order=%d name=%s count=%zu", phase, entry.order, entry.name, g_phase_registry[phase].size);
        entry.fn(phase, entry.data);
    }
}
