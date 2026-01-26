#include "engine/prefab/registry/pf_registry.h"
#include "engine/core/logger/logger.h"

#include <stdlib.h>

/*
Where the operations for parsing perfab components are registered.
Each component has a slot in the registry, and the ops for that
component can be looked up by its enum id.

One handler at a time, engine adds its handlers first,
then game adds its handlers.

Open/Closed Principle: keeps engine modification away from game code,
and allows game to add new handlers without modifying engine code. 
*/
typedef struct {
    bool registered;
    pf_component_ops_t ops;
} pf_registry_slot_t;

static pf_registry_slot_t g_registry[ENUM_COMPONENT_COUNT];

bool pf_register_set(const pf_component_ops_t* ops)
{
    if (!ops) return false;
    if ((int)ops->id < 0 || ops->id >= ENUM_COMPONENT_COUNT) return false;
    if (!ops->apply) return false;
    pf_registry_slot_t* slot = &g_registry[ops->id];
    if (slot->registered) {
        LOGC(LOGCAT_PREFAB, LOG_LVL_FATAL, "prefab registry: duplicate handler for %d", (int)ops->id);
        abort();
    }
    slot->registered = true;
    slot->ops = *ops;
    return true;
}

const pf_component_ops_t* pf_register_get(ComponentEnum id)
{
    if ((int)id < 0 || id >= ENUM_COMPONENT_COUNT) return NULL;
    const pf_registry_slot_t* slot = &g_registry[id];
    return slot->registered ? &slot->ops : NULL;
}
