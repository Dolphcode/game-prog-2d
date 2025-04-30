#ifndef __MINIMAP_H__
#define __MINIMAP_H__

#include "simple_json.h"

#include "gfc_vector.h"

#include "gf2d_graphics.h"

#include "world.h"
#include "entity.h"

#include "ui/window.h"
#include "ui/widget.h"

typedef struct {
	// Sprites and visual info
	Sprite		*border;	// <The minimap border
	Sprite		*tile_layer;	// <The minimap tile_layer
	
	// Minimap drawing info
	int		map_size;	// <The size of the map
	int		tile_size;	// <The number of pixels per tile to be drawn
	GFC_Vector2D	map_dims;	// <The size of the minimap in tiles
	GFC_Vector2D	img_size;	// <The size of the image

	// Binding information
	World		*world;		// <The bound world for this minimap
	GFC_List	*ent_list;	// <The entity list for this minimap
	Entity		*target;	// <The follow target
}W_MinimapData;

void w_minimap_configure(Widget *self, SJson *json);

void w_minimap_draw(Widget *self);

/**
 * @brief this function builds the tile layer of the minimap and hooks the minimap to the player and world
 * @param self the widget to be configured
 * @param world the world to be bound
 * @param player the entity to be targeted
 */
void w_minimap_map_config(Widget *self, World *world, Entity *player);

#endif
