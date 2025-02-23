#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "gfc_vector.h"
#include "gfc_shape.h"

typedef struct {
	// Spatial collision info
	GFC_Vector2D	normal; // <The collision normal vector
	
	// Collider info
	GFC_Shape	*shape;	// <The other shape that participated in the collision

}Collision;

#endif
