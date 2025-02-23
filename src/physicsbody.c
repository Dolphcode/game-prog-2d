#include "simple_logger.h"

#include "physicsbody.h"

PhysicsBody *physics_body_new(Uint32 max_collisions) {
    // Create the body object
    slog("how we doin?");

    slog("the size of which is %i", sizeof(PhysicsBody));
    PhysicsBody *body = gfc_allocate_array(sizeof(PhysicsBody), 1);
    slog("the body has now been allocated, sweet! %i", body);
    if (!body) {
        slog("failed to allocate memory for body");
        return NULL;
    }

    // Assign the max collisions and create the collision list
    body->max_collisions = 8;
    /*
    if (max_collisions) {
        body->collision_list = gfc_allocate_array(sizeof(Collision), max_collisions);
    }*/

    // Return the new physics body
    return body;
}

void physics_body_free(PhysicsBody *self) {
    // Validate the object first
    if (!self) return;

    // Validate and free the collision list
    /*
    if (self->collision_list) {
	slog("freeing our collision list?");
        free(self->collision_list);
	self->collision_list = NULL;
    }*/
    //free(self->collision_list);

    // Free the physics body
    free(self);
}


void physics_body_apply_force(PhysicsBody *self, GFC_Vector2D force) {
	if (!self) return;
	gfc_vector2d_add(self->acceleration, self->acceleration, force);
}
