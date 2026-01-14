#include "engine/ecs/ecs_core.h"
#include "engine/core/logger.h"
#include <string.h>

// =============== ECS Storage =============
ComponentMask   ecs_mask[ECS_MAX_ENTITIES];
uint32_t        ecs_gen[ECS_MAX_ENTITIES];
uint32_t        ecs_next_gen[ECS_MAX_ENTITIES];

// ========== O(1) create/delete ==========
static int free_stack[ECS_MAX_ENTITIES];
static int free_top = 0;
static uint8_t ecs_destroy_state[ECS_MAX_ENTITIES];

enum {
    ECS_DESTROY_NONE = 0,
    ECS_DESTROY_MARKED = 1,
    ECS_DESTROY_CLEANED = 2
};

static ecs_component_hook_fn cmp_on_destroy_table[ENUM_COMPONENT_COUNT];

// =============== Helpers ==================
int ent_index_checked(ecs_entity_t e) {
    return (e.idx < ECS_MAX_ENTITIES && ecs_gen[e.idx] == e.gen && e.gen != 0)
        ? (int)e.idx : -1;
}

int ent_index_unchecked(ecs_entity_t e){ return (int)e.idx; }

bool ecs_alive_idx(int i){ return ecs_gen[i] != 0; }

bool ecs_alive_handle(ecs_entity_t e){ return ent_index_checked(e) >= 0; }

ecs_entity_t handle_from_index(int i){
    return (ecs_entity_t){ (uint32_t)i, ecs_gen[i] };
}

float clampf(float v, float a, float b){
    return (v < a) ? a : ((v > b) ? b : v);
}

static void cmp_on_destroy_noop(int idx)
{
    (void)idx;
}

void ecs_register_component_destroy_hook(ComponentEnum comp, ecs_component_hook_fn fn)
{
    if (comp < 0 || comp >= ENUM_COMPONENT_COUNT) return;
    cmp_on_destroy_table[comp] = fn ? fn : cmp_on_destroy_noop;
}

void ecs_register_phys_body_create_hook(ecs_component_hook_fn fn)
{
    phys_body_create_hook = fn;
}

static void ecs_init_destroy_table(void)
{
    for (int i = 0; i < ENUM_COMPONENT_COUNT; ++i) {
        cmp_on_destroy_table[i] = cmp_on_destroy_noop;
    }
    phys_body_create_hook = NULL;
}

// =============== Public: lifecycle ========
void ecs_init(void){
    memset(ecs_mask, 0, sizeof(ecs_mask));
    memset(ecs_gen,  0, sizeof(ecs_gen));
    memset(ecs_next_gen, 0, sizeof(ecs_next_gen));
    memset(ecs_destroy_state, 0, sizeof(ecs_destroy_state));
    free_top = 0;
    ecs_init_destroy_table();
    for (int i = ECS_MAX_ENTITIES - 1; i >= 0; --i) {
        free_stack[free_top++] = i;
    }

}

void ecs_shutdown(void){
}

static void ecs_cleanup_entity(int idx)
{
    ComponentMask mask = ecs_mask[idx];
    for (int comp = 0; comp < ENUM_COMPONENT_COUNT; ++comp) {
        if (mask & (1ull << comp)) {
            cmp_on_destroy_table[comp](idx);
        }
    }
}

static void ecs_finalize_destroy(int idx)
{
    uint32_t g = ecs_gen[idx];
    g = (g + 1) ? (g + 1) : 1;
    ecs_gen[idx] = 0;
    ecs_next_gen[idx] = g;
    ecs_mask[idx] = 0;
    ecs_destroy_state[idx] = ECS_DESTROY_NONE;
    free_stack[free_top++] = idx;
}

// =============== Public: entity ===========
ecs_entity_t ecs_create(void)
{
    if (free_top == 0) {
        LOGC(LOGCAT_ECS, LOG_LVL_ERROR, "ecs: out of entities (max=%d)", ECS_MAX_ENTITIES);
        return ecs_null();
    }
    int idx = free_stack[--free_top];
    uint32_t g = ecs_next_gen[idx];
    if (g == 0) g = 1;
    ecs_gen[idx] = g;
    ecs_mask[idx] = 0;
    return (ecs_entity_t){ (uint32_t)idx, g };
}

void ecs_destroy(ecs_entity_t e)
{
    int idx = ent_index_checked(e);
    if (idx < 0) return;

    ecs_cleanup_entity(idx);
    ecs_finalize_destroy(idx);
}

void ecs_mark_destroy(ecs_entity_t e)
{
    int idx = ent_index_checked(e);
    if (idx < 0) return;
    if (ecs_destroy_state[idx] == ECS_DESTROY_NONE) {
        ecs_destroy_state[idx] = ECS_DESTROY_MARKED;
    }
}

void ecs_cleanup_marked(void)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (ecs_destroy_state[i] != ECS_DESTROY_MARKED) continue;
        if (!ecs_alive_idx(i)) {
            ecs_destroy_state[i] = ECS_DESTROY_NONE;
            continue;
        }
        ecs_cleanup_entity(i);
        ecs_destroy_state[i] = ECS_DESTROY_CLEANED;
    }
}

void ecs_destroy_marked(void)
{
    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (ecs_destroy_state[i] == ECS_DESTROY_NONE) continue;
        if (!ecs_alive_idx(i)) {
            ecs_destroy_state[i] = ECS_DESTROY_NONE;
            continue;
        }
        if (ecs_destroy_state[i] == ECS_DESTROY_MARKED) {
            ecs_cleanup_entity(i);
        }
        ecs_finalize_destroy(i);
    }
}

ecs_count_result_t ecs_count_entities(const ComponentMask* masks, int num_masks)
{
    ecs_count_result_t result = { .num = num_masks };
    memset(result.count, 0, sizeof(result.count));

    for (int i = 0; i < ECS_MAX_ENTITIES; ++i) {
        if (!ecs_alive_idx(i)) continue;
        ComponentMask mask = ecs_mask[i];
        for (int j = 0; j < num_masks; ++j) {
            if ((mask & masks[j]) == masks[j]) {
                result.count[j]++;
            }
        }
    }
    return result;
}
