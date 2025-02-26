#include "simple_logger.h"

#include "gf2d_sprite.h"
#include "gf2d_graphics.h"

#include "bug.h"
#include "camera.h"

void bug_think(Entity *self) {

}

void bug_update(Entity *self) {
	if (!self) return;
	self->lifetime += 0.1;
	if (self->lifetime > 15) {
		entity_free(self);
		return;
	}

	gfc_vector2d_add(self->position, self->position, self->velocity);
}

Entity *bug_new_entity(GFC_Vector2D position, const char *filename) {
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to spawn a new bug entity");
		return NULL;
	}

	// Copy position data
	gfc_vector2d_copy(self->position, position);

	// Initialize and assign sprite
	entity_configure_from_file(self, filename);

	// Assign functions
	self->think = bug_think;
	self->update = bug_update;

	return self;
}
