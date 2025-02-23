#ifndef __PHYSICSBODY_H__
#define __PHYSICSBODY_H__

#include "gfc_vector.h"
#include "gfc_shape.h"

#include "collision.h"

typedef struct {
	// Management
	Uint8		_inuse;

	// Quantities
	GFC_Vector2D	position;           //<The body's position in space
	GFC_Vector2D	velocity;           //<The body's velocity
	GFC_Vector2D	acceleration;       //<The internally applied acceleration of the physics body
	GFC_Vector2D	net_acceleration;   //<The net acceleration used to integrate velocity

	// Collisions
	GFC_Rect	collider;		//<The collision box for the physics body
	Uint32		max_collisions;         //<The maximum number of collisions that this body can register, 0 if cannot collide
	Uint32		unresolved_collisions;  //<The number of unresolved collisions from a call of the world collision test
	Collision	collision_list[8];        //<The list of collisions, sized max_collisions
}PhysicsBody;

/**
 * @brief create a new physics body with a given number of maximum collisions
 * @param max_collisions that maximum number of collisions this body can detect
 * @return NULL if failed to allocate memory, otherwise a new physics body
 */
PhysicsBody *physics_body_new(Uint32 max_collisions);

/**
 * @brief free a physics body object
 * @param self the object to be freed
 */
void physics_body_free(PhysicsBody *self);

void physics_body_apply_force(PhysicsBody *self, GFC_Vector2D force);

#endif
