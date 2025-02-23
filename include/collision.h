#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "gfc_vector.h"
#include "gfc_shape.h"

typedef struct {
	// Spatial collision info
	GFC_Vector2D	normal; 	// <The collision normal vector
	
	// Keeping track in collision lists
	Uint8		resolved;	// <Track whether the collision was resolved or not

	// Collider info
	GFC_Rect	shape;		// <The other shape that participated in the collision
	float		overlap;	// <the overlap area used to determine which collider to resolve first
}Collision;

#endif
