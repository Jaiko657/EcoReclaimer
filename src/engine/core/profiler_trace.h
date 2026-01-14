#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "shared/utils/build_config.h"

#define PROF_TRACE_ENABLE_JSON  (1 << 0)

#if DEBUG_BUILD

void prof_trace_start(double duration_sec, int flags);
bool prof_trace_stop(const char* out_prefix);
bool prof_trace_is_active(void);

void prof_trace_frame_begin(uint32_t frame_id);
void prof_trace_frame_end(uint32_t frame_id);
void prof_trace_tick_begin(uint32_t frame_id);
void prof_trace_tick_end(uint32_t frame_id);
void prof_trace_present_begin(uint32_t frame_id);
void prof_trace_present_end(uint32_t frame_id);
void prof_trace_present_end_frame(uint32_t frame_id);
void prof_trace_phase_begin(int tid, uint32_t frame_id, const char* phase_name);
void prof_trace_phase_end(int tid, uint32_t frame_id);
void prof_trace_system_begin(int tid, uint32_t frame_id, const char* system_name);
void prof_trace_system_end(int tid, uint32_t frame_id);

#else

static inline void prof_trace_start(double duration_sec, int flags)
{ (void)duration_sec; (void)flags; }

static inline bool prof_trace_stop(const char* out_prefix)
{ (void)out_prefix; return false; }

static inline bool prof_trace_is_active(void)
{ return false; }

static inline void prof_trace_frame_begin(uint32_t frame_id)
{ (void)frame_id; }

static inline void prof_trace_frame_end(uint32_t frame_id)
{ (void)frame_id; }

static inline void prof_trace_tick_begin(uint32_t frame_id)
{ (void)frame_id; }

static inline void prof_trace_tick_end(uint32_t frame_id)
{ (void)frame_id; }

static inline void prof_trace_present_begin(uint32_t frame_id)
{ (void)frame_id; }

static inline void prof_trace_present_end(uint32_t frame_id)
{ (void)frame_id; }

static inline void prof_trace_present_end_frame(uint32_t frame_id)
{ (void)frame_id; }

static inline void prof_trace_phase_begin(int tid, uint32_t frame_id, const char* phase_name)
{ (void)tid; (void)frame_id; (void)phase_name; }

static inline void prof_trace_phase_end(int tid, uint32_t frame_id)
{ (void)tid; (void)frame_id; }

static inline void prof_trace_system_begin(int tid, uint32_t frame_id, const char* system_name)
{ (void)tid; (void)frame_id; (void)system_name; }

static inline void prof_trace_system_end(int tid, uint32_t frame_id)
{ (void)tid; (void)frame_id; }

#endif
