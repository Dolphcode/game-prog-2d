#ifndef __PHYSICSBODY_H__
#define __PHYSICSBODY_H__

#include "gfc_vector.h"
#include "gfc_shape.h"

#include "collision.h"

typedef struct {
	// Quantities
	GFC_Vector2D	position;           //<The body's position in space
	GFC_Vector2D	velocity;           //<The body's velocity
	GFC_Vector2D	acceleration;       //<The internally applied acceleration of the physics body
	GFC_Vector2D	net_acceleration;   //<The net acceleration used to integrate velocity

	// Collisions
	GFC_Rect	collider;		//<The collision box for the physics body
	Uint32		max_collisions;         //<The maximum number of collisions that this body can register, 0 if cannot collide
	Uint32		unresolved_collisions;  //<The number of unresolved collisions from a call of the world collision test
	Collision	*collision_list;        //<The list of collisions, sized max_collisions
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

/**
 * @brief applies a force to a physics body
 * @param self the body which a force should be applied to
 * @param force the force to be applied
 */
void physics_body_apply_force(PhysicsBody *self, GFC_Vector2D force);

/**
 * @brief applies an impulse (impulse is converted to acceleration via p/t = F)
 * @param self the body which an impulse should be applied to
 * @parma impulse the impulse vector to be applied
 */
void physics_body_apply_impulse(PhysicsBody *self, GFC_Vector2D impulse);
#endif
