#include "shared/utils/build_config.h"

#if DEBUG_BUILD

#include "engine/renderer/renderer.h"
#include "engine/asset/asset.h"
#include "engine/runtime/toast.h"
#include "engine/input/input.h"
#include "engine/runtime/camera.h"
#include "engine/core/logger/logger.h"
#include "engine/debug/profile_trace/profiler_trace.h"
#include "shared/actions.h"
#include "engine/debug/debug_hotkeys/debug_hotkeys.h"
#include "engine/engine/engine_manager/engine_manager.h"
#include "engine/ecs/ecs_core.h"
#include "engine/ecs/ecs_engine.h"
#include "engine/debug/debug_str/debug_str_registry.h"
#include "engine/world/world_query.h"
#include "engine/engine/engine_scheduler/engine_scheduler_registration.h"
#include "engine/core/platform/platform.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

static bool debug_trace_next_prefix(char* out_prefix, size_t cap)
{
    const char* dir = "captures";
    if (!platform_dir_exists(dir)) {
        if (!platform_make_dir(dir)) {
            return false;
        }
    }

    static int s_trace_next = 0;
    for (int attempt = 0; attempt < 10000; ++attempt) {
        int idx = s_trace_next + attempt;
        snprintf(out_prefix, cap, "%s/trace_%05d", dir, idx);

        char path[512];
        snprintf(path, sizeof(path), "%s_trace.json", out_prefix);
        if (platform_file_exists(path)) continue;

        s_trace_next = idx + 1;
        return true;
    }

    return false;
}

void sys_debug_binds(const input_t* in)
{
    if (input_pressed(in, ACT_ASSET_DEBUG_PRINT)) {
        asset_reload_all();
        asset_log_debug();
        ui_toast(1.0f, "Assets reloaded");
    }

    if (input_pressed(in, ACT_DEBUG_COLLIDER_ECS)) {
        bool on = renderer_toggle_ecs_colliders();
        ui_toast(1.0f, "ECS colliders: %s", on ? "on" : "off");
    }

    if (input_pressed(in, ACT_DEBUG_COLLIDER_PHYSICS)) {
        bool on = renderer_toggle_phys_colliders();
        ui_toast(1.0f, "Physics colliders: %s", on ? "on" : "off");
    }

    if (input_pressed(in, ACT_DEBUG_COLLIDER_STATIC)) {
        bool on = renderer_toggle_static_colliders();
        ui_toast(1.0f, "Static colliders: %s", on ? "on" : "off");
    }

    if (input_pressed(in, ACT_DEBUG_TRIGGERS)) {
        bool on = renderer_toggle_triggers();
        ui_toast(1.0f, "Triggers: %s", on ? "on" : "off");
    }

    if (input_pressed(in, ACT_DEBUG_RELOAD_TMX)) {
        bool ok = engine_reload_world();
        int w = 0, h = 0;
        if (!world_size_tiles(&w, &h)) {
            LOGC(LOGCAT_WORLD, LOG_LVL_WARN, "debug_hotkeys: world size unavailable");
            w = 0;
            h = 0;
        }
        ui_toast(1.0f, "TMX reload: %s (%dx%d tiles)", ok ? "ok" : "failed", w, h);
    }

    static bool s_inspect_mode = false;
    if (input_pressed(in, ACT_DEBUG_INSPECT)) {
        s_inspect_mode = !s_inspect_mode;
        ui_toast(1.0f, "Inspect mode: %s", s_inspect_mode ? "on" : "off");
    }

    if (s_inspect_mode && input_pressed(in, ACT_MOUSE_L)) {
        float world_x = 0.0f;
        float world_y = 0.0f;
        (void)renderer_screen_to_world(in->mouse.x, in->mouse.y, &world_x, &world_y);

        int best = -1;
        float best_d2 = 0.0f;
        for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
            if (!ecs_alive_idx(i)) continue;
            ComponentMask mask = ecs_mask[i];
            if ((mask & CMP_POS) == 0) continue;

            float cx = cmp_pos[i].x;
            float cy = cmp_pos[i].y;
            float hx = 8.0f;
            float hy = 8.0f;

            if (mask & CMP_COL) {
                if (cmp_col[i].hx > 0.0f) hx = cmp_col[i].hx;
                if (cmp_col[i].hy > 0.0f) hy = cmp_col[i].hy;
            } else if (mask & CMP_SPR) {
                float w = fabsf(cmp_spr[i].src.w);
                float h = fabsf(cmp_spr[i].src.h);
                if (w > 0.0f) hx = w * 0.5f;
                if (h > 0.0f) hy = h * 0.5f;
            }

            if (world_x < cx - hx || world_x > cx + hx ||
                world_y < cy - hy || world_y > cy + hy) {
                continue;
            }

            float dx = world_x - cx;
            float dy = world_y - cy;
            float d2 = dx*dx + dy*dy;
            if (best < 0 || d2 < best_d2) {
                best = i;
                best_d2 = d2;
            }
        }

        if (best >= 0) {
            ecs_entity_t h = handle_from_index(best);
            ComponentMask mask = ecs_mask[best];
            LOGC(LOGCAT_ECS, LOG_LVL_INFO,
                 "[inspect] entity=%u gen=%u mask=0x%016" PRIX64 " pos=(%.1f, %.1f) click_world=(%.1f, %.1f)",
                 h.idx, h.gen, (uint64_t)mask, cmp_pos[best].x, cmp_pos[best].y, world_x, world_y);

            const char* cmp_indent = "  ";
            for (int comp = 0; comp < ENUM_COMPONENT_COUNT; ++comp) {
                ComponentMask bit = 1ull << comp;
                if ((mask & bit) == 0) continue;
                char info[256];
                if (debug_str_component((ComponentEnum)comp, h, info, sizeof(info))) {
                    LOGC(LOGCAT_ECS, LOG_LVL_INFO, "%s%s", cmp_indent, info);
                } else {
                    const char* name = component_name_from_id((ComponentEnum)comp);
                    if (name) {
                        LOGC(LOGCAT_ECS, LOG_LVL_INFO, "%s%s()", cmp_indent, name);
                    }
                }
            }
        } else {
            LOGC(LOGCAT_ECS, LOG_LVL_INFO,
                 "[inspect] click_world=(%.1f, %.1f) hit none", world_x, world_y);
        }
    }

    if (input_pressed(in, ACT_DEBUG_FPS)) {
        bool on = renderer_toggle_fps_overlay();
        ui_toast(1.0f, "FPS overlay: %s", on ? "on" : "off");
    }

    if (input_pressed(in, ACT_DEBUG_TRACE_START)) {
        if (prof_trace_is_active()) {
            ui_toast(1.0f, "Trace already active");
        } else {
            prof_trace_start(0.0, PROF_TRACE_ENABLE_JSON);
            ui_toast(1.0f, "Trace started");
        }
    }

    if (input_pressed(in, ACT_DEBUG_TRACE_STOP)) {
        if (!prof_trace_is_active()) {
            ui_toast(1.0f, "Trace not active");
        } else {
            char prefix[256];
            if (!debug_trace_next_prefix(prefix, sizeof(prefix))) {
                ui_toast(1.0f, "Trace failed: no free filename");
            } else {
                if (!prof_trace_stop(prefix)) {
                    ui_toast(1.0f, "Trace failed: write error");
                } else {
                    ui_toast(1.0f, "Saved trace: %s_trace.json", prefix);
                }
            }
        }
    }
}

void debug_post_frame(void)
{
    const input_t* in = input_frame_snapshot();
    if (!in || !input_pressed(in, ACT_DEBUG_SCREENSHOT)) return;

    const char* dir = "screenshots";
    if (!platform_dir_exists(dir)) {
        if (!platform_make_dir(dir)) {
            ui_toast(2.0f, "Screenshot failed: can't create '%s'", dir);
            return;
        }
    }

    static int s_next = 0;
    char path[512];
    for (int attempt = 0; attempt < 10000; ++attempt) {
        int idx = s_next + attempt;
        snprintf(path, sizeof(path), "%s/screenshot_%05d.png", dir, idx);
        if (platform_file_exists(path)) continue;

        if (!platform_take_screenshot(path)) {
            ui_toast(2.0f, "Screenshot failed: capture error");
            return;
        }
        s_next = idx + 1;
        ui_toast(2.0f, "Saved screenshot: %s", path);
        return;
    }

    ui_toast(2.0f, "Screenshot failed: no free filename");
}

SYSTEMS_ADAPT_INPUT(sys_debug_binds_adapt, sys_debug_binds)

#endif // DEBUG_BUILD
