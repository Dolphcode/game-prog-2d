#ifndef __TILEDATA_H__
#define __TILEDATA_H__

#include "gfc_vector.h"

typedef enum {
	NONE = 0,	// <No collision, player will pass through block
	FULL = 1,	// <Full collision, block will stop player
	ONE_WAY = 2	// <One-way collision, will only collide with player if player is colliding from above
}TileCollisionType;

typedef struct {
	// Draw data
	Uint32			frame;	// <Which frame this tile represents
	
	// Collision data
	TileCollisionType	collision_type;	// <The type of collision this tile
	GFC_Vector2D		collision_box;	// <The tile's bounding box size
}TileData;

/**
 * @brief tiledata is a shared resource, call this to free the resource
 * @param data the data container to be freed
 */

/**
 * @brief allocate memory for tile data
 * @return NULL if failed to allocate memory, otherwise a blank tiledata object
 */

#endif
