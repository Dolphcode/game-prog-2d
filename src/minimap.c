#include "simple_logger.h"

#include "gf2d_sprite.h"
#include "gf2d_draw.h"

#include "ui/minimap.h"

/*
typedef struct {
	Sprite		*border;	// <The minimap border
	Sprite		*tile_layer;	// <The minimap tile_layer

	World		*world;		// <The bound world for this minimap
	GFC_List	*ent_list;	// <The entity list for this minimap
	Entity		*target;	// <The follow target
}W_MinimapData;
*/

void w_minimap_data_free(Widget *self) {
	if (!self) return;
	W_MinimapData *data = (W_MinimapData *)self->data;
	if (!data) return;

	if (data->border) gf2d_sprite_free(data->border);
	if (data->tile_layer) gf2d_sprite_free(data->tile_layer);
}

void w_minimap_configure(Widget *self, SJson *json) {
	if (!self || !json) return;
	W_MinimapData *data = calloc(1, sizeof(W_MinimapData));

	self->data = data;
	self->data_free = w_minimap_data_free;
}

void w_minimap_draw(Widget *self) {

}

void w_minimap_map_config(Widget *self, World *world, Entity *player) {
	if (!self || !world || !player) return;
	W_MinimapData *data = (W_MinimapData *)self->data;
	if (!data) return;
	
	// Bind the world and player
	data->world = world;
	data->ent_list = world->entity_list;
	data->target = world->player;

	// Draw the tile layer sprite
}


