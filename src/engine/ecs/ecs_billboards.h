#pragma once

#include <stdbool.h>

typedef bool (*ecs_billboard_filter_fn)(int trigger_idx, int matched_idx, void* data);

void ecs_billboards_clear_filters(void);
void ecs_billboards_add_filter(ecs_billboard_filter_fn fn, void* data);
