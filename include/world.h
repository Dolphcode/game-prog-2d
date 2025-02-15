#ifndef __WORLD_H__
#define __WORLD_H__

#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_vector.h"

#include "gf2d_sprite.h"

#include "camera.h"
#include "entity.h"
#include "tiledata.h"

typedef struct
{
	// Object metadata
	GFC_TextLine	name;

	// World background sprites
	Sprite		*background;	// <The static background image
	Sprite		*foreground;	// <The parallax foreground image
	float		bg_factor;	// <The parallax factor
	
	// Tileset config
	Sprite		*tile_set;	// <The tileset sprite of this world
	Uint32		tile_size;	// <The width and height of tiles in the tileset
	
	// Spatial config
	GFC_Vector2D	world_size;	// <The width and height of the world in pixels
	Uint32		tile_count;	// <The number of tiles in the tileset
	TileData*	tile_data;	// <An array of tile data, reserve 0 for air tiles
	Uint32*		tile_map;	// <The map of tiles in the level
	
	// Tile layer(s)
	Sprite		*tile_layer;	// <The sprite which tiles will be drawn onto

	// Entities and the main camera
	Camera		*main_camera;	// <The camera object corresponding with this world
	GFC_List	*entity_list;	// <The list of entities in the list
}World;

/**
 * @brief set the active world object
 * @param world a pointer to a World object to be made the active world
 */
void world_make_active(World *world);

/**
 * @brief get the active world object
 * @return NULL if there is no active world, or the active world pointer otherwise
 */
World *world_get_active();

/**
 * @brief frees a world object
 * @param world the world object to be freed
 */
void world_free(World *world);

/**
 * @brief allocates memory to create a new world object
 * @param width the width of the world in tiles
 * @param height the height of the world in tiles
 * @param tile_count the number of tile types in the tileset
 * @return NULL if fail, otherwise a blank world object
 */
World *world_new(Uint32 width, Uint32 height, Uint32 tile_count);

/**
 * @brief loads a world object from a filename
 * @param filename the path to the def file for the world we are loading
 * @return NULL if fail, otherwise the world object being loaded
 */
World *world_load(const char *filename);

/**
 * @brief draws the world's tilemap
 * @param world the world object to be drawn
 */
void world_draw(World *world);
#endif
