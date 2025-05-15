#include <SDL.h>
#include <SDL_image.h>

#include "simple_logger.h"

#include "gfc_config.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "gf2d_draw.h"

#include "ui/minimap.h"

/*
typedef struct {
	// Sprites and visual info
	Sprite		*border;	// <The minimap border
	Sprite		*tile_layer;	// <The minimap tile_layer
	
	// Minimap drawing info
	int		map_size;	// <The size of the map
	int		tile_size;	// <The number of pixels per tile to be drawn
	GFC_Vector2D	map_size;	// <The size of the minimap
	GFC_Vector2D	img_size;	// <The size of the image

	// Binding information
	World		*world;		// <The bound world for this minimap
	GFC_List	*ent_list;	// <The entity list for this minimap
	Entity		*target;	// <The follow target
}W_MinimapData;*/



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

	// Load the data object
	SJson *data_obj = sj_object_get_value(json, "data");
	sj_object_get_int(data_obj, "mapSize", &(data->map_size));
	sj_object_get_int(data_obj, "tileSize", &(data->tile_size));

	// Get the border
	char *sprite = sj_object_get_string(data_obj, "borderSprite");
	if (sprite) {
		char sprite_path[256];
		strcpy(sprite_path, sprite);
		GFC_Vector2D frame_size = {0};
		Uint32 fpl = 0;
		sj_object_get_vector2d(data_obj, "borderSpriteSize", &frame_size);
		sj_object_get_uint32(data_obj, "borderSpriteFPL", &fpl);
		data->border = gf2d_sprite_load_all(
			sprite,
			(Uint32)frame_size.x,
			(Uint32)frame_size.y,
			fpl,
			0);
	}

	// Set the draw function
	self->draw = w_minimap_draw;

	self->data = data;
	self->data_free = w_minimap_data_free;
}

void w_minimap_draw(Widget *self) {
	if (!self) return;
	W_MinimapData *data = (W_MinimapData *)self->data;
	if (!data || !data->tile_layer) return;

	// Compute the draw rect
	int center = data->tile_size / 2;
	int offset = data->map_size / 2;

	int player_tile_x = (int)(data->target->position.x / data->world->tile_size * data->tile_size) + center - offset;
	int player_tile_y = (int)(data->target->position.y / data->world->tile_size * data->tile_size) + center - offset;

	GFC_Vector4D clip_rect = {(float)(player_tile_x + data->map_size) / data->img_size.x, 
		(float)(player_tile_y + data->map_size) / data->img_size.y, 
		(float)data->map_size / data->img_size.x + (float)(player_tile_x + data->map_size) / data->img_size.x, 
		(float)data->map_size / data->img_size.y + (float)(player_tile_y + data->map_size) / data->img_size.y};
	
	
	GFC_Vector2D draw_pos;
	GFC_Vector2D border_offset = gfc_vector2d((data->border->frame_w - data->map_size) / 2, (data->border->frame_h - data->map_size) / 2);
	gfc_vector2d_sub(draw_pos, self->position, border_offset);
	gf2d_sprite_draw(data->border,
			draw_pos,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			0);

	// Compute the offset rect
	GFC_Vector2D draw_offset = gfc_vector2d(clip_rect.x * data->img_size.x, clip_rect.y * data->img_size.y);
	draw_pos = self->position;
	gfc_vector2d_sub(draw_pos, draw_pos, draw_offset);

	gf2d_sprite_render(data->tile_layer,
			draw_pos,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			&clip_rect,
			0);

	// Draw enemy locations
	GFC_List *ents = data->world->entity_list;
	int c = gfc_list_count(ents);
	for (int i = 0; i < c; i++) {
		Entity *ent = gfc_list_get_nth(ents, i);
		if (!ent) continue;
		if (!ent->alive) continue;
		
		// Compute the relative position to the player
		GFC_Vector2D ent_offset;
		gfc_vector2d_sub(ent_offset, ent->position, data->target->position);
		ent_offset.x = (int)(ent_offset.x / data->world->tile_size * data->tile_size);
		ent_offset.y = (int)(ent_offset.y / data->world->tile_size * data->tile_size);
		if (ent_offset.x < -data->map_size / 2 || ent_offset.x > data->map_size / 2 ||
				ent_offset.y < -data->map_size / 2 || ent_offset.y > data->map_size / 2) continue;

		ent_offset.x += self->position.x + data->map_size / 2;
		ent_offset.y += self->position.y + data->map_size / 2;
		
		if (ent->icon_sprite) {
			gf2d_sprite_draw(
				ent->icon_sprite,
				ent_offset,
				NULL,
				&ent->icon_offset,
				NULL,
				NULL,
				NULL,
				0);
		} /*else {
			gf2d_draw_circle(ent_offset, 10, GFC_COLOR_ORANGE);
		}*/
		
	}




}

void w_minimap_map_config(Widget *self, World *world, Entity *player) {
	if (!self || !world || !player) return;
	W_MinimapData *data = (W_MinimapData *)self->data;
	if (!data) return;
	
	// Bind the world and player
	data->world = world;
	data->ent_list = world->entity_list;
	data->target = world->player;

	// Delete the pre existing tile layer if need be then create the new sprite
	if (data->tile_layer) gf2d_sprite_free(data->tile_layer);
	data->tile_layer = gf2d_sprite_new();

	// Compute tile info
	data->map_dims = gfc_vector2d(world->world_size.x, world->world_size.y);
	data->img_size = gfc_vector2d(data->map_dims.x * data->tile_size + 2 * data->map_size, data->map_dims.y * data->tile_size + 2 * data->map_size);

	// Get the renderer and build the texture for the map
	GFC_Vector2D screen_res = gf2d_graphics_get_resolution();
	SDL_Renderer *renderer = gf2d_graphics_get_renderer();
	SDL_Texture *minimap = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_TARGET,
			data->img_size.x, data->img_size.y);
	SDL_SetRenderTarget(renderer, minimap);

	// Draw the background
	SDL_Rect bg_rect = {0, 0, data->img_size.x, data->img_size.y};
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &bg_rect);

	// Begin drawing the minimap
	for (int row = 0; row < data->map_dims.y; ++row) {
		for (int col = 0; col < data->map_dims.x; ++col) {
			// Check if its an air tile
			int index = world->world_size.x * row + col;
			if (world->tile_map[index] == 0) continue;

			// Get the color
			TileData dat = world->tile_data[world->tile_map[index] - 1];
			GFC_Color tile_color = dat.minimap_color;
			
			// Compute the rect position
			SDL_Rect rect = {col * data->tile_size + (data->map_size), row * data->tile_size + (data->map_size), data->tile_size, data->tile_size};

			// Now draw the rect with the correct color
			SDL_SetRenderDrawColor(renderer, tile_color.r, tile_color.g, tile_color.b, 255);
			SDL_RenderFillRect(renderer, &rect);
		}
	}
	
	// Make a surface out of the texture
	SDL_Surface *surface = gf2d_graphics_create_surface(data->img_size.x, data->img_size.y);
	SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch); 
	
	// Assign everything to the sprite
	data->tile_layer->texture = minimap;
	data->tile_layer->surface = surface;
	data->tile_layer->frames_per_line = 1;
	data->tile_layer->frame_w = data->img_size.x;
	data->tile_layer->frame_h = data->img_size.y;

	// Return the renderer to the screen
	SDL_SetRenderTarget(renderer, NULL);
}


