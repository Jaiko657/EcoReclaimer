#include "game/ui/ui.h"
#include "engine/renderer/renderer.h"

#include <stddef.h>

void game_ui_layer_hud(const render_view_t* view, void* data);
void game_ui_layer_grav_gun(const render_view_t* view, void* data);

void game_ui_register_layers(void)
{
    renderer_ui_register_layer(game_ui_layer_hud, NULL, 0);
    renderer_ui_register_layer(game_ui_layer_grav_gun, NULL, 10);
}
