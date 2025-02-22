#ifndef __BODY_H__
#define __BODY_H__

#include "gfc_vector.h"
#include "gfc_shape.h"

typedef struct {
	// Physics quantities
	GFC_Vector2D	position; 		//<the body's position in space
	GFC_Vector2D	velocity; 		//<the body's velocity in space
	GFC_Vector2D	acceleration;		//<the body's acceleration modified by the entity object and cleared at the end of the frame
	GFC_Vector2D	net_acceleration;	//<the body's total acceleration actually used for calculation
	
	// Collision config
	GFC_Shape	collider;		//<the body's collider
}Body;

/**
 * @brief allocate memory for a new physics body
 * @return NULL if failed to allocate, otherwise a blank physics body
 */
Body *body_new();

/**
 * @brief free a physics body object
 * @param self the body object to be freed
 */
void body_free(Body *self);

#endif
