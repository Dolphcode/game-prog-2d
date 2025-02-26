#include "simple_logger.h"

#include "physicsbody.h"

PhysicsBody *physics_body_new(Uint32 max_collisions) {
    // Create the body object

    PhysicsBody *body = gfc_allocate_array(sizeof(PhysicsBody), 1);
    if (!body) {
        slog("failed to allocate memory for body");
        return NULL;
    }

    // Assign the max collisions and create the collision list
    body->max_collisions = 8;

    // Return the new physics body
    return body;
}

void physics_body_free(PhysicsBody *self) {
    // Validate the object first
    if (!self) return;

    self->ent = NULL;

    // Free the physics body
    free(self);
}


void physics_body_apply_force(PhysicsBody *self, GFC_Vector2D force) {
	if (!self) return;
	gfc_vector2d_add(self->acceleration, self->acceleration, force);
}
