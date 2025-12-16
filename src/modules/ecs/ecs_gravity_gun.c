#include "modules/ecs/ecs_internal.h"
#include "modules/ecs/ecs_render.h"
#include "modules/ecs/ecs_proximity.h"
#include "modules/core/input.h"
#include "modules/core/toast.h"
#include "modules/systems/systems_registration.h"
#include "modules/core/effects.h"
#include "modules/asset/asset.h"
#include "modules/renderer/renderer.h"
#include <float.h>
#include <math.h>

enum { GUN_CHARGER_FLASH_TIME_MS = 750 };
static const float GUN_CHARGER_FLASH_TIME = 0.001f * (float)GUN_CHARGER_FLASH_TIME_MS;
static const float GUN_CHARGER_EJECT_DURATION = 0.30f;
static const float GUN_CHARGER_EJECT_SPEED = 60.0f;

static void sprite_clear_component(int idx);

static bool grav_gun_get_mouse_world(const input_t* in, v2f* out)
{
    if (!in || !out) return false;
    return renderer_screen_to_world(in->mouse.x, in->mouse.y, &out->x, &out->y);
}

static void grav_gun_destroy_hook(int idx)
{
    ecs_entity_t gun = handle_from_index(idx);
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_PLAYER) == 0) continue;
        if (cmp_player[i].held_gun.idx == gun.idx && cmp_player[i].held_gun.gen == gun.gen) {
            cmp_player[i].held_gun = ecs_null();
        }
    }
}

void ecs_register_grav_gun_component_hooks(void)
{
    ecs_register_component_destroy_hook(ENUM_GRAV_GUN, grav_gun_destroy_hook);
}

static void liftable_destroy_hook(int idx)
{
    ecs_entity_t liftable = handle_from_index(idx);
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_PLAYER) == 0) continue;
        if (cmp_player[i].held_liftable.idx == liftable.idx &&
            cmp_player[i].held_liftable.gen == liftable.gen) {
            cmp_player[i].held_liftable = ecs_null();
        }
    }
}

void ecs_register_liftable_component_hooks(void)
{
    ecs_register_component_destroy_hook(ENUM_LIFTABLE, liftable_destroy_hook);
}

static int player_held_gun_index(ecs_entity_t player)
{
    int player_idx = ent_index_checked(player);
    if (player_idx < 0 || (ecs_mask[player_idx] & CMP_PLAYER) == 0) return -1;

    ecs_entity_t gun = cmp_player[player_idx].held_gun;
    int gun_idx = ent_index_checked(gun);
    if (gun_idx < 0 || (ecs_mask[gun_idx] & CMP_GRAV_GUN) == 0) {
        cmp_player[player_idx].held_gun = ecs_null();
        return -1;
    }
    if (!cmp_grav_gun[gun_idx].held) {
        cmp_player[player_idx].held_gun = ecs_null();
        return -1;
    }
    if (cmp_grav_gun[gun_idx].holder.idx != player.idx ||
        cmp_grav_gun[gun_idx].holder.gen != player.gen) {
        cmp_player[player_idx].held_gun = ecs_null();
        return -1;
    }
    return gun_idx;
}

static int player_held_liftable_index(ecs_entity_t player)
{
    int player_idx = ent_index_checked(player);
    if (player_idx < 0 || (ecs_mask[player_idx] & CMP_PLAYER) == 0) return -1;

    ecs_entity_t liftable = cmp_player[player_idx].held_liftable;
    int liftable_idx = ent_index_checked(liftable);
    if (liftable_idx < 0 || (ecs_mask[liftable_idx] & CMP_LIFTABLE) == 0) {
        cmp_player[player_idx].held_liftable = ecs_null();
        return -1;
    }
    if (cmp_liftable[liftable_idx].state != GRAV_GUN_STATE_HELD ||
        cmp_liftable[liftable_idx].holder.idx != player.idx ||
        cmp_liftable[liftable_idx].holder.gen != player.gen) {
        cmp_player[player_idx].held_liftable = ecs_null();
        return -1;
    }
    return liftable_idx;
}

static int find_grav_gun_near_player(ecs_entity_t player)
{
    ecs_prox_iter_t it = ecs_prox_stay_begin();
    ecs_prox_view_t v;
    while (ecs_prox_stay_next(&it, &v)) {
        int trigger_idx = ent_index_checked(v.trigger_owner);
        int matched_idx = ent_index_checked(v.matched_entity);
        if (trigger_idx < 0 || matched_idx < 0) continue;
        if ((ecs_mask[trigger_idx] & CMP_GRAV_GUN) == 0) continue;
        if (cmp_grav_gun[trigger_idx].held) continue;
        if (v.matched_entity.idx == player.idx && v.matched_entity.gen == player.gen) {
            return trigger_idx;
        }
    }
    return -1;
}

static int find_gun_charger_near_player(ecs_entity_t player)
{
    ecs_prox_iter_t it = ecs_prox_stay_begin();
    ecs_prox_view_t v;
    while (ecs_prox_stay_next(&it, &v)) {
        int trigger_idx = ent_index_checked(v.trigger_owner);
        if (trigger_idx < 0) continue;
        if ((ecs_mask[trigger_idx] & CMP_GUN_CHARGER) == 0) continue;
        if (v.matched_entity.idx == player.idx && v.matched_entity.gen == player.gen) {
            return trigger_idx;
        }
    }
    return -1;
}

static bool charger_can_accept(int charger_idx)
{
    if (charger_idx < 0 || (ecs_mask[charger_idx] & CMP_GUN_CHARGER) == 0) return false;
    return !ecs_alive_handle(cmp_gun_charger[charger_idx].stored_gun);
}

static void clear_charger_for_gun(ecs_entity_t gun)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_GUN_CHARGER) == 0) continue;
        if (cmp_gun_charger[i].stored_gun.idx == gun.idx && cmp_gun_charger[i].stored_gun.gen == gun.gen) {
            cmp_gun_charger[i].stored_gun = ecs_null();
            cmp_gun_charger[i].flash_timer = 0.0f;
            sprite_clear_component(i);
        }
    }
}

static void swap_player_sprite_texture(int player_idx, const char* path)
{
    if (player_idx < 0) return;
    if ((ecs_mask[player_idx] & CMP_SPR) == 0) return;
    tex_handle_t new_tex = asset_acquire_texture(path);
    if (asset_texture_valid(cmp_spr[player_idx].tex)) {
        asset_release_texture(cmp_spr[player_idx].tex);
    }
    cmp_spr[player_idx].tex = new_tex;
}

static void sprite_clear_component(int idx)
{
    if (idx < 0 || (ecs_mask[idx] & CMP_SPR) == 0) return;
    if (asset_texture_valid(cmp_spr[idx].tex)) {
        asset_release_texture(cmp_spr[idx].tex);
    }
    cmp_spr[idx] = (cmp_sprite_t){0};
    ecs_mask[idx] &= ~CMP_SPR;
}

static void charger_assume_gun_sprite(int charger_idx, int gun_idx)
{
    if (charger_idx < 0 || gun_idx < 0) return;
    if ((ecs_mask[gun_idx] & CMP_SPR) == 0) return;
    sprite_clear_component(charger_idx);
    cmp_add_sprite_handle(handle_from_index(charger_idx),
                          cmp_spr[gun_idx].tex,
                          cmp_spr[gun_idx].src,
                          cmp_spr[gun_idx].ox,
                          cmp_spr[gun_idx].oy);
}

static void tool_pickup(int gun_idx, ecs_entity_t player, int player_idx)
{
    clear_charger_for_gun(handle_from_index(gun_idx));
    cmp_grav_gun[gun_idx].held = true;
    cmp_grav_gun[gun_idx].holder = player;
    cmp_grav_gun[gun_idx].toast_pending = false;
    if (ecs_mask[player_idx] & CMP_PLAYER) {
        cmp_player[player_idx].held_gun = handle_from_index(gun_idx);
    }
    if ((ecs_mask[gun_idx] & CMP_POS) != 0) {
        cmp_pos[gun_idx].x = cmp_pos[player_idx].x;
        cmp_pos[gun_idx].y = cmp_pos[player_idx].y;
        ecs_mask[gun_idx] &= ~CMP_POS;
    }
    swap_player_sprite_texture(player_idx, "assets/images/character_withgun.png");
}

static void tool_drop(int gun_idx, int player_idx)
{
    cmp_grav_gun[gun_idx].held = false;
    cmp_grav_gun[gun_idx].holder = ecs_null();
    if (ecs_mask[player_idx] & CMP_PLAYER) {
        cmp_player[player_idx].held_gun = ecs_null();
    }
    cmp_add_position((ecs_entity_t){ (uint32_t)gun_idx, ecs_gen[gun_idx] },
                     cmp_pos[player_idx].x, cmp_pos[player_idx].y);
    swap_player_sprite_texture(player_idx, "assets/images/character.png");
}

static void charger_eject_gun(int charger_idx, int gun_idx)
{
    if (charger_idx < 0 || gun_idx < 0) return;
    if (ecs_mask[charger_idx] & CMP_POS) {
        cmp_add_position((ecs_entity_t){ (uint32_t)gun_idx, ecs_gen[gun_idx] },
                         cmp_pos[charger_idx].x, cmp_pos[charger_idx].y);
        cmp_grav_gun[gun_idx].eject_timer = GUN_CHARGER_EJECT_DURATION;
        if ((ecs_mask[gun_idx] & CMP_GRAV_GUN) != 0) {
            cmp_grav_gun_t* gun = &cmp_grav_gun[gun_idx];
            if (gun->charge >= gun->max_charge) {
                gun->toast_pending = true;
            }
        }
    }
    cmp_gun_charger[charger_idx].stored_gun = ecs_null();
    cmp_gun_charger[charger_idx].flash_timer = 0.0f;
    sprite_clear_component(charger_idx);
}

static void tool_place_in_charger(int gun_idx, int player_idx, int charger_idx)
{
    cmp_grav_gun[gun_idx].held = false;
    cmp_grav_gun[gun_idx].holder = ecs_null();
    if (ecs_mask[player_idx] & CMP_PLAYER) {
        cmp_player[player_idx].held_gun = ecs_null();
    }
    if (ecs_mask[gun_idx] & CMP_POS) {
        ecs_mask[gun_idx] &= ~CMP_POS;
    }
    charger_assume_gun_sprite(charger_idx, gun_idx);
    cmp_gun_charger[charger_idx].stored_gun = handle_from_index(gun_idx);
    cmp_gun_charger[charger_idx].flash_timer = 0.0f;
    if (cmp_grav_gun[gun_idx].charge >= cmp_grav_gun[gun_idx].max_charge) {
        cmp_gun_charger[charger_idx].flash_timer = GUN_CHARGER_FLASH_TIME;
    }
    swap_player_sprite_texture(player_idx, "assets/images/character.png");
}

static void sys_grav_gun_charger_impl(float dt)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_GUN_CHARGER) == 0) continue;

        ecs_entity_t stored = cmp_gun_charger[i].stored_gun;
        if (!ecs_alive_handle(stored)) {
            cmp_gun_charger[i].stored_gun = ecs_null();
            cmp_gun_charger[i].flash_timer = 0.0f;
            sprite_clear_component(i);
            continue;
        }

        int gun_idx = ent_index_checked(stored);
        if (gun_idx < 0 || (ecs_mask[gun_idx] & CMP_GRAV_GUN) == 0) {
            cmp_gun_charger[i].stored_gun = ecs_null();
            cmp_gun_charger[i].flash_timer = 0.0f;
            sprite_clear_component(i);
            continue;
        }

        if (cmp_gun_charger[i].flash_timer > 0.0f) {
            cmp_gun_charger[i].flash_timer -= dt;
            if (cmp_gun_charger[i].flash_timer <= 0.0f) {
                charger_eject_gun(i, gun_idx);
            }
            continue;
        }

        cmp_grav_gun_t* gun = &cmp_grav_gun[gun_idx];
        if (gun->max_charge <= 0.0f) continue;

        if (gun->charge < gun->max_charge) {
            float rate = gun->regen_rate;
            if (rate <= 0.0f) rate = 0.25f;
            gun->charge = clampf(gun->charge + rate * dt, 0.0f, gun->max_charge);
            gun->toast_pending = false;
        }

        if (gun->charge >= gun->max_charge && !gun->toast_pending) {
            cmp_gun_charger[i].flash_timer = 2.0f * GUN_CHARGER_FLASH_TIME;
            gun->toast_pending = true;
        }
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        cmp_grav_gun_t* gun = &cmp_grav_gun[i];
        if (gun->eject_timer <= 0.0f) continue;
        if ((ecs_mask[i] & CMP_POS) == 0) continue;
        cmp_pos[i].y += GUN_CHARGER_EJECT_SPEED * dt;
        gun->eject_timer -= dt;
        if (gun->eject_timer <= 0.0f) {
            gun->eject_timer = 0.0f;
            if (gun->toast_pending) {
                ui_toast(2.0f, "Charged!");
                gun->toast_pending = false;
            }
        }
    }
}
static bool grav_gun_hit_test(int idx, float mx, float my, float pad)
{
    if ((ecs_mask[idx] & CMP_POS) == 0) return false;
    const float cx = cmp_pos[idx].x;
    const float cy = cmp_pos[idx].y;

    if (ecs_mask[idx] & CMP_COL) {
        float hx = cmp_col[idx].hx + pad;
        float hy = cmp_col[idx].hy + pad;
        return (mx >= cx - hx && mx <= cx + hx && my >= cy - hy && my <= cy + hy);
    }

    const float dx = mx - cx;
    const float dy = my - cy;
    return (dx * dx + dy * dy) <= (pad * pad);
}

static int find_grab_candidate(int player_idx, v2f mouse_world)
{
    const float px = cmp_pos[player_idx].x;
    const float py = cmp_pos[player_idx].y;

    float best_d2 = FLT_MAX;
    int best_idx = -1;

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (i == player_idx) continue;
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & (CMP_LIFTABLE | CMP_POS | CMP_PHYS_BODY)) != (CMP_LIFTABLE | CMP_POS | CMP_PHYS_BODY)) continue;

        cmp_liftable_t* g = &cmp_liftable[i];
        if (g->state != GRAV_GUN_STATE_FREE) continue;

        if (cmp_phys_body[i].type == PHYS_STATIC) continue;

        const float pickup_dist = (g->pickup_distance > 0.0f) ? g->pickup_distance : 48.0f;
        const float dxp = cmp_pos[i].x - px;
        const float dyp = cmp_pos[i].y - py;
        if ((dxp * dxp + dyp * dyp) > (pickup_dist * pickup_dist)) continue;

        const float pad = (g->pickup_radius > 0.0f) ? g->pickup_radius : 8.0f;
        if (!grav_gun_hit_test(i, mouse_world.x, mouse_world.y, pad)) continue;

        const float dxm = cmp_pos[i].x - mouse_world.x;
        const float dym = cmp_pos[i].y - mouse_world.y;
        const float d2 = dxm * dxm + dym * dym;
        if (d2 < best_d2) {
            best_d2 = d2;
            best_idx = i;
        }
    }

    return best_idx;
}

static void grav_gun_set_player_filter(int idx, bool ignore_player)
{
    if ((ecs_mask[idx] & CMP_PHYS_BODY) == 0) return;

    cmp_liftable_t* g = &cmp_liftable[idx];
    if (ignore_player) {
        if (!g->saved_mask_valid) {
            g->saved_mask_bits = cmp_phys_body[idx].mask_bits;
            g->saved_mask_valid = true;
        }
        unsigned int mask = cmp_phys_body[idx].mask_bits;
        if (mask == 0u) mask = 0xFFFFFFFFu;
        mask &= ~PHYS_CAT_PLAYER;
        if (ecs_mask[idx] & CMP_RESOURCE) {
            mask &= ~PHYS_CAT_TARDAS;
        }
        cmp_phys_body[idx].mask_bits = mask;
    } else if (g->saved_mask_valid) {
        cmp_phys_body[idx].mask_bits = g->saved_mask_bits;
        g->saved_mask_valid = false;
    }
}

static void begin_hold(int idx, ecs_entity_t holder, v2f mouse_world)
{
    cmp_liftable_t* g = &cmp_liftable[idx];
    g->holder = holder;
    g->state = GRAV_GUN_STATE_HELD;
    g->just_dropped = false;
    g->grab_offset_x = cmp_pos[idx].x - mouse_world.x;
    g->grab_offset_y = cmp_pos[idx].y - mouse_world.y;
    g->hold_vel_x = 0.0f;
    g->hold_vel_y = 0.0f;
    int holder_idx = ent_index_checked(holder);
    if (holder_idx >= 0 && (ecs_mask[holder_idx] & CMP_PLAYER)) {
        cmp_player[holder_idx].held_liftable = handle_from_index(idx);
    }

    if ((ecs_mask[idx] & CMP_VEL) == 0) {
        cmp_add_velocity(handle_from_index(idx), 0.0f, 0.0f, DIR_SOUTH);
    } else {
        cmp_vel[idx].x = 0.0f;
        cmp_vel[idx].y = 0.0f;
    }

    grav_gun_set_player_filter(idx, true);
}

static void release_hold(int idx)
{
    cmp_liftable_t* g = &cmp_liftable[idx];
    ecs_entity_t holder = g->holder;
    int holder_idx = ent_index_checked(holder);
    if (holder_idx >= 0 && (ecs_mask[holder_idx] & CMP_PLAYER)) {
        cmp_player[holder_idx].held_liftable = ecs_null();
    }
    g->holder = ecs_null();
    g->state = GRAV_GUN_STATE_FREE;
    g->just_dropped = true;
    g->hold_vel_x = 0.0f;
    g->hold_vel_y = 0.0f;
    grav_gun_set_player_filter(idx, false);
}

static void sys_grav_gun_input_impl(const input_t* in)
{
    if (!in) return;

    ecs_entity_t player = find_player_handle();
    int player_idx = ent_index_checked(player);
    if (player_idx < 0 || (ecs_mask[player_idx] & CMP_POS) == 0) return;
    int tool_idx = player_held_gun_index(player);
    if (tool_idx < 0) return;
    if (cmp_grav_gun[tool_idx].charge <= 0.0f) {
        ui_toast(1.0f, "No Charge, Recharge at Station");
        return;
    }

    int held_idx = player_held_liftable_index(player);
    if (held_idx >= 0) {
        if (!input_down(in, BTN_MOUSE_L)) {
            release_hold(held_idx);
        }
        return;
    }

    if (!input_pressed(in, BTN_MOUSE_L)) return;

    v2f mouse_world = v2f_make(0.0f, 0.0f);
    if (!grav_gun_get_mouse_world(in, &mouse_world)) return;

    int pickup_idx = find_grab_candidate(player_idx, mouse_world);
    if (pickup_idx >= 0) {
        begin_hold(pickup_idx, player, mouse_world);
    }
}

static void sys_grav_gun_tool_impl(const input_t* in)
{
    if (!in || !input_pressed(in, BTN_INTERACT)) return;

    ecs_entity_t player = find_player_handle();
    int player_idx = ent_index_checked(player);
    if (player_idx < 0 || (ecs_mask[player_idx] & CMP_POS) == 0) return;

    int held_tool_idx = player_held_gun_index(player);
    if (held_tool_idx >= 0) {
        int held_resource_idx = player_held_liftable_index(player);
        if (held_resource_idx >= 0) {
            release_hold(held_resource_idx);
        }
        int charger_idx = find_gun_charger_near_player(player);
        if (charger_idx >= 0 && charger_can_accept(charger_idx)) {
            tool_place_in_charger(held_tool_idx, player_idx, charger_idx);
            return;
        }
        tool_drop(held_tool_idx, player_idx);
        return;
    }

    int pickup_idx = find_grav_gun_near_player(player);
    if (pickup_idx >= 0) {
        tool_pickup(pickup_idx, player, player_idx);
    }
}

static void update_held(int idx, cmp_liftable_t* g, float dt, const input_t* in)
{
    int holder_idx = ent_index_checked(g->holder);
    if (holder_idx < 0 || (ecs_mask[holder_idx] & CMP_POS) == 0) {
        release_hold(idx);
        return;
    }

    int tool_idx = player_held_gun_index(g->holder);
    if (tool_idx < 0 || cmp_grav_gun[tool_idx].charge <= 0.0f) {
        release_hold(idx);
        return;
    }

    if (!in || !input_down(in, BTN_MOUSE_L)) {
        release_hold(idx);
        return;
    }

    const float breakoff = g->breakoff_distance;
    if (breakoff > 0.0f) {
        const float dx = cmp_pos[idx].x - cmp_pos[holder_idx].x;
        const float dy = cmp_pos[idx].y - cmp_pos[holder_idx].y;
        const float dist2 = dx * dx + dy * dy;
        if (dist2 > breakoff * breakoff) {
            release_hold(idx);
            return;
        }
    }

    v2f mouse_world = v2f_make(0.0f, 0.0f);
    if (!grav_gun_get_mouse_world(in, &mouse_world)) return;

    float target_x = mouse_world.x + g->grab_offset_x;
    float target_y = mouse_world.y + g->grab_offset_y;

    const float px = cmp_pos[holder_idx].x;
    const float py = cmp_pos[holder_idx].y;
    const float max_hold = g->max_hold_distance;
    if (max_hold > 0.0f) {
        float dx = target_x - px;
        float dy = target_y - py;
        float dist2 = dx * dx + dy * dy;
        float max2 = max_hold * max_hold;
        if (dist2 > max2 && dist2 > 0.0001f) {
            float dist = sqrtf(dist2);
            float scale = max_hold / dist;
            target_x = px + dx * scale;
            target_y = py + dy * scale;
        }
    }

    const float ex = target_x - cmp_pos[idx].x;
    const float ey = target_y - cmp_pos[idx].y;

    float desired_x = ex * g->follow_gain;
    float desired_y = ey * g->follow_gain;

    float max_speed = g->max_speed;
    if (max_speed > 0.0f) {
        float speed2 = desired_x * desired_x + desired_y * desired_y;
        float max2 = max_speed * max_speed;
        if (speed2 > max2 && speed2 > 0.0001f) {
            float speed = sqrtf(speed2);
            float scale = max_speed / speed;
            desired_x *= scale;
            desired_y *= scale;
        }
    }

    float blend = clampf(g->damping * dt, 0.0f, 1.0f);
    g->hold_vel_x = g->hold_vel_x + (desired_x - g->hold_vel_x) * blend;
    g->hold_vel_y = g->hold_vel_y + (desired_y - g->hold_vel_y) * blend;

    if ((ecs_mask[idx] & CMP_VEL) == 0) {
        cmp_add_velocity(handle_from_index(idx), g->hold_vel_x, g->hold_vel_y, DIR_SOUTH);
    } else {
        cmp_vel[idx].x = g->hold_vel_x;
        cmp_vel[idx].y = g->hold_vel_y;
    }
}

static void sys_grav_gun_motion_impl(float dt, const input_t* in)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_GRAV_GUN) == 0) continue;

        cmp_grav_gun_t* gun = &cmp_grav_gun[i];
        if (gun->max_charge <= 0.0f) continue;

        bool draining = false;
        if (gun->held && ecs_alive_handle(gun->holder)) {
            int holder_idx = ent_index_checked(gun->holder);
            if (holder_idx >= 0 && (ecs_mask[holder_idx] & CMP_PLAYER)) {
                ecs_entity_t liftable = cmp_player[holder_idx].held_liftable;
                int liftable_idx = ent_index_checked(liftable);
                if (liftable_idx >= 0 &&
                    (ecs_mask[liftable_idx] & CMP_LIFTABLE) != 0 &&
                    cmp_liftable[liftable_idx].state == GRAV_GUN_STATE_HELD) {
                    draining = true;
                } else {
                    cmp_player[holder_idx].held_liftable = ecs_null();
                }
            }
        }

        float delta = draining ? -gun->drain_rate : 0.0f;
        gun->charge = clampf(gun->charge + delta * dt, 0.0f, gun->max_charge);
        if (gun->charge < gun->max_charge) {
            gun->toast_pending = false;
        }
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_LIFTABLE) == 0) continue;

        cmp_liftable_t* g = &cmp_liftable[i];
        if (g->state == GRAV_GUN_STATE_HELD) {
            update_held(i, g, dt, in);
        }
    }
}

static void sys_grav_gun_fx_impl(void)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & (CMP_LIFTABLE | CMP_POS)) != (CMP_LIFTABLE | CMP_POS)) continue;

        cmp_liftable_t* g = &cmp_liftable[i];
        if (g->state != GRAV_GUN_STATE_HELD) continue;

        int holder_idx = ent_index_checked(g->holder);
        if (holder_idx < 0 || (ecs_mask[holder_idx] & CMP_POS) == 0) continue;

        colorf color = (colorf){ 0.470588f, 0.784314f, 1.0f, 1.0f };
        int thickness = 1;
        if (ecs_mask[i] & CMP_SPR) {
            cmp_spr[i].fx.highlighted = true;
            color = cmp_spr[i].fx.highlight_base_color;
            if (color.a <= 0.0f) {
                color = cmp_spr[i].fx.highlight_color;
            }
            thickness = cmp_spr[i].fx.highlight_thickness;
            cmp_spr[i].fx.front = true;
        }
        if (thickness < 1) thickness = 1;

        int tool_idx = player_held_gun_index(g->holder);
        if (tool_idx >= 0) {
            cmp_grav_gun_t* tool = &cmp_grav_gun[tool_idx];
            if (tool->max_charge > 0.0f) {
                float ratio = tool->charge / tool->max_charge;
                if (ratio < 0.3f) {
                    float t = (0.3f - ratio) / 0.3f;
                    if (t < 0.0f) t = 0.0f;
                    if (t > 1.0f) t = 1.0f;
                    colorf low = (colorf){ 1.0f, 0.2f, 0.2f, 1.0f };
                    color.r = color.r + (low.r - color.r) * t;
                    color.g = color.g + (low.g - color.g) * t;
                    color.b = color.b + (low.b - color.b) * t;
                }
            }
        }

        if (ecs_mask[i] & CMP_SPR) {
            cmp_spr[i].fx.highlight_color = color;
        }

        v2f start = v2f_make(cmp_pos[holder_idx].x, cmp_pos[holder_idx].y);
        v2f end = v2f_make(cmp_pos[i].x, cmp_pos[i].y);
        fx_line_push(start, end, (float)thickness, color);
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_GUN_CHARGER) == 0) continue;
        if ((ecs_mask[i] & CMP_SPR) == 0) continue;
        if (!ecs_alive_handle(cmp_gun_charger[i].stored_gun)) continue;
        cmp_spr[i].fx.front = true;
        if (cmp_gun_charger[i].flash_timer <= 0.0f) continue;
        cmp_spr[i].fx.highlighted = true;
        cmp_spr[i].fx.highlight_color = (colorf){ 0.2f, 1.0f, 0.2f, 1.0f };
    }

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        if ((ecs_mask[i] & CMP_GRAV_GUN) == 0) continue;
        if ((ecs_mask[i] & CMP_SPR) == 0) continue;
        if (cmp_grav_gun[i].eject_timer <= 0.0f) continue;
        cmp_spr[i].fx.front = true;
    }
}

SYSTEMS_ADAPT_INPUT(sys_grav_gun_input_adapt, sys_grav_gun_input_impl)
SYSTEMS_ADAPT_INPUT(sys_grav_gun_tool_adapt, sys_grav_gun_tool_impl)
SYSTEMS_ADAPT_BOTH(sys_grav_gun_motion_adapt, sys_grav_gun_motion_impl)
SYSTEMS_ADAPT_DT(sys_grav_gun_charger_adapt, sys_grav_gun_charger_impl)
SYSTEMS_ADAPT_VOID(sys_grav_gun_fx_adapt, sys_grav_gun_fx_impl)
