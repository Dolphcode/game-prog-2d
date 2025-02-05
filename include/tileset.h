#ifndef __TILESET_H__
#define __TILESET_H__

#include "gfc_text.h"

#include "gf2d_sprite.h"

typedef struct {
	
	GFC_TextLine	name;
	Sprite		*tiles;
	GFC_Vector2D	tileSize;

}TileSet;

#endif
