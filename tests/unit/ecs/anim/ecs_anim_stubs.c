#include "game/ecs/ecs_game.h"
#include "engine/core/logger/logger.h"

ComponentMask   ecs_mask[ECS_MAX_ENTITIES];
uint32_t        ecs_gen[ECS_MAX_ENTITIES];
uint32_t        ecs_next_gen[ECS_MAX_ENTITIES];

static ecs_entity_t g_player = {0, 0};

void ecs_anim_stub_set_player(ecs_entity_t e)
{
    g_player = e;
}

int ent_index_checked(ecs_entity_t e)
{
    return (e.idx < ECS_MAX_ENTITIES && ecs_gen[e.idx] == e.gen && e.gen != 0) ? (int)e.idx : -1;
}

bool ecs_alive_idx(int i)
{
    return ecs_gen[i] != 0;
}

ecs_entity_t find_player_handle(void)
{
    return g_player;
}

bool log_would_log(log_level_t lvl)
{
    (void)lvl;
    return true;
}

void log_msg(log_level_t lvl, const log_cat_t* cat, const char* fmt, ...)
{
    (void)lvl; (void)cat; (void)fmt;
}
