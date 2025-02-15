#ifndef __WORLD_H__
#define __WORLD_H__

#include "gfc_list.h"
#include "gfc_text.h"

#include "gf2d_sprite.h"

#include "camera.h"
#include "entity.h"
#include "world.h"

typedef struct
{
	// Object metadata
	GFC_TextLine	name;

	// World background sprites
	Sprite		*background;	// <The static background image
	Sprite		*foreground;	// <The parallax foreground image
	float		bg_factor;	// <The parallax factor

	// References
	Camera		*main_camera;	// <The camera object corresponding with this world
	GFC_List	*entity_list;	// <The list of entities in the list
}World;

/**
 * @brief frees a world object
 * @param the world object to be freed
 */
void world_free(World *world);

/**
 * @brief allocates memory to create a new world object
 * @return NULL if fail, otherwise a blank world object
 */
World *world_new();

/**
 * @brief loads a world object from a filename
 * @param filename the path to the def file for the world we are loading
 * @return NULL if fail, otherwise the world object being loaded
 */
World *world_load(const char *filename);
#endif
