#ifndef __WORLD_H__
#define __WORLD_H__

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_list.h"

#include "gf2d_sprite.h"

typedef struct
{
	// Debug fields
	GFC_TextLine	name;
	
	// Visuals
	Sprite 		*background;
	Sprite 		*tileSet;

	// World Config
	GFC_Vector2D	size;

	// Tilemap Config
	GFC_Vector2D	tileMapSize;
	Uint8		*tileMap;

	// Entity List
	GFC_List	*entity_list;
}World;

#endif
