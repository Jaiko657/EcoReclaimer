#include "engine/engine/engine_scheduler/engine_scheduler.h"
#include "engine/core/logger/logger.h"
#include "engine/debug/profile_trace/profiler_trace.h"
#include "engine/utils/dynarray.h"
#include <stdbool.h>
#include <string.h>

typedef struct {
    const char* name;
    int order;
    systems_fn fn;
} sys_rec_t;

static DA(sys_rec_t) g_systems[PHASE_COUNT] = {0};
static size_t    g_counts[PHASE_COUNT];
static uint32_t  g_frame_id = 0;
static bool      g_frame_open = false;

static int lane_for_phase(systems_phase_t phase)
{
    return (phase == PHASE_PRE_RENDER || phase == PHASE_RENDER) ? 2 : 1;
}

static const char* phase_name(systems_phase_t phase)
{
    switch (phase) {
    case PHASE_INPUT: return "PHASE_INPUT";
    case PHASE_SIM_PRE: return "PHASE_SIM_PRE";
    case PHASE_PHYSICS: return "PHASE_PHYSICS";
    case PHASE_SIM_POST: return "PHASE_SIM_POST";
    case PHASE_DEBUG: return "PHASE_DEBUG";
    case PHASE_PRE_RENDER: return "PHASE_PRE_RENDER";
    case PHASE_RENDER: return "PHASE_RENDER";
    default: return "PHASE_UNKNOWN";
    }
}

static uint32_t engine_scheduler_frame_begin_if_needed(void)
{
    if (!g_frame_open) {
        ++g_frame_id;
        g_frame_open = true;
        prof_trace_frame_begin(g_frame_id);
    }
    return g_frame_id;
}

static void sort_phase(systems_phase_t phase)
{
    size_t n = g_counts[phase];
    for (size_t i = 1; i < n; ++i) {
        sys_rec_t key = g_systems[phase].data[i];
        size_t j = i;
        while (j > 0 && g_systems[phase].data[j - 1].order > key.order) {
            g_systems[phase].data[j] = g_systems[phase].data[j - 1];
            --j;
        }
        g_systems[phase].data[j] = key;
    }
}

void engine_scheduler_init(void)
{
    for (int p = 0; p < (int)PHASE_COUNT; ++p) {
        DA_CLEAR(&g_systems[p]);
        g_counts[p] = 0;
    }
    g_frame_id = 0;
    g_frame_open = false;
}

void engine_scheduler_register(systems_phase_t phase, int order, systems_fn fn, const char* name)
{
    if ((int)phase < 0 || phase >= PHASE_COUNT) {
        LOGC(LOGCAT(SYS), LOG_LVL_WARN, "systems: invalid phase %d for %s", phase, name ? name : "(unnamed)");
        return;
    }
    size_t* cnt = &g_counts[phase];
    DA_APPEND(&g_systems[phase], ((sys_rec_t){ .name = name, .order = order, .fn = fn }));
    *cnt = g_systems[phase].size;
    sort_phase(phase);
}

void engine_scheduler_run_phase(systems_phase_t phase, float dt, const input_t* in)
{
    if ((int)phase < 0 || phase >= PHASE_COUNT) return;
    size_t n = g_counts[phase];
    int tid = lane_for_phase(phase);
    uint32_t frame = g_frame_id;
    prof_trace_phase_begin(tid, frame, phase_name(phase));
    for (size_t i = 0; i < n; ++i) {
        systems_fn fn = g_systems[phase].data[i].fn;
        if (fn) {
            const char* sys_name = g_systems[phase].data[i].name;
            if (!sys_name) sys_name = "(unnamed)";
            prof_trace_system_begin(tid, frame, sys_name);
            fn(dt, in);
            prof_trace_system_end(tid, frame);
        }
    }
    prof_trace_phase_end(tid, frame);
}

bool engine_scheduler_get_phase_systems(systems_phase_t phase, const systems_info_t** out_list, size_t* out_count)
{
    if (out_list) *out_list = NULL;
    if (out_count) *out_count = 0;
    if (!out_list || !out_count) {
        LOGC(LOGCAT(SYS), LOG_LVL_WARN, "engine_scheduler_get_phase_systems: missing out params");
        return false;
    }
    if ((int)phase < 0 || phase >= PHASE_COUNT) {
        LOGC(LOGCAT(SYS), LOG_LVL_WARN, "engine_scheduler_get_phase_systems: invalid phase %d", phase);
        return false;
    }
    *out_list = (const systems_info_t*)g_systems[phase].data;
    *out_count = g_counts[phase];
    return true;
}

void engine_scheduler_tick(float dt, const input_t* in)
{
    uint32_t frame = engine_scheduler_frame_begin_if_needed();
    prof_trace_tick_begin(frame);
    engine_scheduler_run_phase(PHASE_INPUT,    dt, in);
    engine_scheduler_run_phase(PHASE_SIM_PRE,  dt, in);
    engine_scheduler_run_phase(PHASE_PHYSICS,  dt, in);
    engine_scheduler_run_phase(PHASE_SIM_POST, dt, in);
    engine_scheduler_run_phase(PHASE_DEBUG,    dt, in);
    prof_trace_tick_end(frame);
}

void engine_scheduler_present(float frame_dt)
{
    uint32_t frame = engine_scheduler_frame_begin_if_needed();
    prof_trace_present_begin(frame);
    engine_scheduler_run_phase(PHASE_PRE_RENDER, frame_dt, NULL);
    engine_scheduler_run_phase(PHASE_RENDER, frame_dt, NULL);
    prof_trace_present_end_frame(frame);
    g_frame_open = false;
}
