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
    body->max_collisions = max_collisions;
    if (max_collisions) {
        body->collision_list = gfc_allocate_array(sizeof(Collision), max_collisions);
    }

    // Return the new physics body
    return body;
}

void physics_body_free(PhysicsBody *self) {
    // Validate the object first
    if (!self) return;

    // Validate and free the collision list
    if (self->collision_list) {
        free(self->collision_list);
    }

    // Free the physics body
    free(self);
}
