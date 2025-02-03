#include "gfc_vector.h"

#include "simple_logger.h"

#include "gf2d_graphics.h"

#include "bug.h"

void bug_think(Entity* self) {
	gfc_vector2d_add(self->position, self->position, self->velocity);
	self->velocity = gfc_vector2d_rotate(self->velocity, 0.01);

}

Entity *bug_new_entity(GFC_Vector2D position) {
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to spawn a new bug entity");
		return NULL;
	}

	// Copy position data
	gfc_vector2d_copy(self->position, position);

	// Initialize and assign sprite
	self->sprite = gf2d_sprite_load_all(
		"images/space_bug.png",
		128,
		128,
		16,
		0);

	// Initialize velocity (for uniform circular motion)
	self->velocity = gfc_vector2d(2, 0);
	
	// Assign the think function
	self->think = bug_think;

	return self;
}
