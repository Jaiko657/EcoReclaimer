
#if DEBUG_BUILD

#include "engine/debug/profile_trace/profiler_trace.h"
#include "engine/core/time/time.h"
#include "engine/core/logger/logger.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#endif

#define PROF_TRACE_CAP 200000
#define PROF_TRACE_STACK_CAP 256

typedef struct {
    const char* name;
    const char* cat;
    uint64_t ts_us;
    uint64_t dur_us;
    int tid;
    uint32_t frame_id;
} prof_trace_event_t;

typedef struct {
    const char* name;
    const char* cat;
    uint64_t start_us;
    uint32_t frame_id;
} prof_trace_stack_entry_t;

static prof_trace_event_t g_events[PROF_TRACE_CAP];
static size_t g_event_count = 0;

static prof_trace_stack_entry_t g_stack_tick[PROF_TRACE_STACK_CAP];
static size_t g_stack_tick_count = 0;
static prof_trace_stack_entry_t g_stack_present[PROF_TRACE_STACK_CAP];
static size_t g_stack_present_count = 0;

static bool g_active = false;
static uint64_t g_end_us = 0;
static uint64_t g_base_us = 0;
static int g_flags = 0;
static int g_auto_index = 0;
static bool g_tick_active = false;
static uint64_t g_tick_start_us = 0;

static uint64_t prof_trace_now_us(void)
{
    double now = time_now();
    if (now <= 0.0) return 0;
    return (uint64_t)(now * 1000000.0);
}

static size_t* prof_trace_stack_count_for_tid(int tid)
{
    return (tid == 2) ? &g_stack_present_count : &g_stack_tick_count;
}

static prof_trace_stack_entry_t* prof_trace_stack_for_tid(int tid)
{
    return (tid == 2) ? g_stack_present : g_stack_tick;
}

static void prof_trace_event_record(const char* name, const char* cat, uint64_t ts_us,
                                    uint64_t dur_us, int tid, uint32_t frame_id)
{
    if (g_event_count >= PROF_TRACE_CAP) return;
    g_events[g_event_count++] = (prof_trace_event_t){
        .name = name,
        .cat = cat,
        .ts_us = ts_us,
        .dur_us = dur_us,
        .tid = tid,
        .frame_id = frame_id,
    };
}

static void prof_trace_push(int tid, const char* cat, const char* name, uint32_t frame_id)
{
    if (!g_active || (g_flags & PROF_TRACE_ENABLE_JSON) == 0) return;

    size_t* count = prof_trace_stack_count_for_tid(tid);
    if (*count >= PROF_TRACE_STACK_CAP) return;

    prof_trace_stack_entry_t* stack = prof_trace_stack_for_tid(tid);
    uint64_t now_us = prof_trace_now_us();
    if (now_us < g_base_us) now_us = g_base_us;

    stack[*count] = (prof_trace_stack_entry_t){
        .name = name ? name : "(unnamed)",
        .cat = cat,
        .start_us = now_us - g_base_us,
        .frame_id = frame_id,
    };
    (*count)++;
}

static void prof_trace_pop(int tid)
{
    if (!g_active || (g_flags & PROF_TRACE_ENABLE_JSON) == 0) return;

    size_t* count = prof_trace_stack_count_for_tid(tid);
    if (*count == 0) return;

    prof_trace_stack_entry_t* stack = prof_trace_stack_for_tid(tid);
    prof_trace_stack_entry_t entry = stack[*count - 1];
    (*count)--;

    uint64_t now_us = prof_trace_now_us();
    if (now_us < g_base_us) now_us = g_base_us;
    uint64_t end_us = now_us - g_base_us;
    uint64_t dur_us = end_us >= entry.start_us ? (end_us - entry.start_us) : 0;

    prof_trace_event_record(entry.name, entry.cat, entry.start_us, dur_us, tid, entry.frame_id);
}

static void prof_trace_write_json(FILE* f)
{
    fprintf(f, "{\n  \"traceEvents\": [\n");

    for (size_t i = 0; i < g_event_count; ++i) {
        const prof_trace_event_t* ev = &g_events[i];
        fprintf(f, "    {");
        fprintf(f, "\"name\":\"");
        const char* name = ev->name ? ev->name : "";
        for (const char* p = name; *p; ++p) {
            if (*p == '\\' || *p == '"') {
                fputc('\\', f);
                fputc(*p, f);
            } else if (*p == '\n') {
                fputs("\\n", f);
            } else if (*p == '\r') {
                fputs("\\r", f);
            } else if (*p == '\t') {
                fputs("\\t", f);
            } else {
                fputc(*p, f);
            }
        }
        fprintf(f, "\",\"cat\":\"%s\",\"ph\":\"X\",", ev->cat ? ev->cat : "");
        fprintf(f, "\"ts\":%" PRIu64 ",\"dur\":%" PRIu64 ",", ev->ts_us, ev->dur_us);
        fprintf(f, "\"pid\":0,\"tid\":%d,", ev->tid);
        fprintf(f, "\"args\":{\"frame\":%u}", ev->frame_id);
        fprintf(f, "}%s\n", (i + 1 < g_event_count) ? "," : "");
    }

    fprintf(f, "  ]\n}\n");
}

static void prof_trace_reset(void)
{
    g_event_count = 0;
    g_stack_tick_count = 0;
    g_stack_present_count = 0;
    g_tick_active = false;
    g_tick_start_us = 0;
}

static bool prof_trace_write_file(const char* out_prefix)
{
    if (!out_prefix || !*out_prefix) {
        LOGC(LOGCAT(PROF), LOG_LVL_WARN, "prof_trace_write_file: empty output prefix");
        return false;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s_trace.json", out_prefix);

    FILE* f = fopen(path, "w");
    if (!f) {
        LOGC(LOGCAT(PROF), LOG_LVL_ERROR, "prof_trace_write_file: failed to open '%s'", path);
        return false;
    }
    prof_trace_write_json(f);
    fclose(f);
    return true;
}

static void prof_trace_stop_auto(void)
{
    struct stat st;
    if (stat("captures", &st) != 0) {
#if defined(_WIN32)
        _mkdir("captures");
#else
        mkdir("captures", 0755);
#endif
    }

    char prefix[256];
    int start = g_auto_index;
    for (int attempt = 0; attempt < 10000; ++attempt) {
        int idx = start + attempt;
        snprintf(prefix, sizeof(prefix), "captures/trace_auto_%05d", idx);
        char path[512];
        snprintf(path, sizeof(path), "%s_trace.json", prefix);

        FILE* f = fopen(path, "r");
        if (f) {
            fclose(f);
            continue;
        }

        g_auto_index = idx + 1;
        prof_trace_write_file(prefix);
        return;
    }
}

static void prof_trace_maybe_autostop(void)
{
    if (!g_active || g_end_us == 0) return;

    uint64_t now_us = prof_trace_now_us();
    if (now_us >= g_end_us) {
        prof_trace_stop_auto();
        g_active = false;
        prof_trace_reset();
    }
}

void prof_trace_start(double duration_sec, int flags)
{
    g_flags = flags;
    prof_trace_reset();
    g_base_us = prof_trace_now_us();
    g_end_us = (duration_sec > 0.0)
        ? (g_base_us + (uint64_t)(duration_sec * 1000000.0))
        : 0;
    g_active = true;
}

bool prof_trace_stop(const char* out_prefix)
{
    if (!g_active) {
        LOGC(LOGCAT(PROF), LOG_LVL_DEBUG, "prof_trace_stop: trace not active");
        return false;
    }
    bool ok = true;
    if ((g_flags & PROF_TRACE_ENABLE_JSON) != 0) {
        ok = prof_trace_write_file(out_prefix);
    }
    g_active = false;
    prof_trace_reset();
    return ok;
}

bool prof_trace_is_active(void)
{
    return g_active;
}

void prof_trace_frame_begin(uint32_t frame_id)
{
    (void)frame_id;
}

void prof_trace_frame_end(uint32_t frame_id)
{
    (void)frame_id;
    prof_trace_maybe_autostop();
}

void prof_trace_tick_begin(uint32_t frame_id)
{
    (void)frame_id;
    if (!g_active || (g_flags & PROF_TRACE_ENABLE_JSON) == 0) return;
    uint64_t now_us = prof_trace_now_us();
    if (now_us < g_base_us) now_us = g_base_us;
    g_tick_start_us = now_us - g_base_us;
    g_tick_active = true;
}

void prof_trace_tick_end(uint32_t frame_id)
{
    if (!g_active || !g_tick_active || (g_flags & PROF_TRACE_ENABLE_JSON) == 0) return;
    uint64_t now_us = prof_trace_now_us();
    if (now_us < g_base_us) now_us = g_base_us;
    uint64_t end_us = now_us - g_base_us;
    uint64_t dur_us = end_us >= g_tick_start_us ? (end_us - g_tick_start_us) : 0;
    prof_trace_event_record("tick", "tick", g_tick_start_us, dur_us, 1, frame_id);
    g_tick_active = false;
    prof_trace_maybe_autostop();
}

void prof_trace_present_begin(uint32_t frame_id)
{
    prof_trace_push(2, "present", "present", frame_id);
}

void prof_trace_present_end(uint32_t frame_id)
{
    (void)frame_id;
    prof_trace_pop(2);
    prof_trace_maybe_autostop();
}

void prof_trace_present_end_frame(uint32_t frame_id)
{
    prof_trace_present_end(frame_id);
    prof_trace_frame_end(frame_id);
}

void prof_trace_phase_begin(int tid, uint32_t frame_id, const char* phase_name)
{
    prof_trace_push(tid, "phase", phase_name, frame_id);
}

void prof_trace_phase_end(int tid, uint32_t frame_id)
{
    (void)frame_id;
    prof_trace_pop(tid);
}

void prof_trace_system_begin(int tid, uint32_t frame_id, const char* system_name)
{
    prof_trace_push(tid, "system", system_name, frame_id);
}

void prof_trace_system_end(int tid, uint32_t frame_id)
{
    (void)frame_id;
    prof_trace_pop(tid);
}

#endif
