#include "engine/prefab/registry/pf_registry.h"
#include "engine/prefab/components/pf_components_engine.h"

void pf_register_engine_components(void)
{
    pf_register_set(pf_component_pos_ops());
    pf_register_set(pf_component_vel_ops());
    pf_register_set(pf_component_phys_body_ops());
    pf_register_set(pf_component_spr_ops());
    pf_register_set(pf_component_anim_ops());
    pf_register_set(pf_component_col_ops());
    pf_register_set(pf_component_trigger_ops());
    pf_register_set(pf_component_billboard_ops());
}
