#include "simple_logger.h"

#include "gf2d_sprite.h"
#include "gf2d_graphics.h"

#include "bug.h"
#include "camera.h"

void bug_think(Entity *self) {

}

void bug_update(Entity *self) {
	if (!self) return;
	slog("bug can update?");
	slog("bug %i", self);
	self->lifetime += 0.1;
	slog("we can update our lifetime?");
	if (self->lifetime > 15) {
		entity_free(self);
		return;
	}
	slog ("and we can check it too");

	gfc_vector2d_add(self->position, self->position, self->velocity);
	slog ("and we can add our position");
	
}

Entity *bug_new_entity(GFC_Vector2D position, const char *filename) {
	Entity *self;
	slog ("here?");
	self = entity_new();
	if (!self) {
		slog("failed to spawn a new bug entity");
		return NULL;
	}

	// Copy position data
	gfc_vector2d_copy(self->position, position);

	// Initialize and assign sprite
	entity_configure_from_file(self, filename);
	slog("made me a new bug from file");

	// Assign functions
	self->think = bug_think;
	self->update = bug_update;

	return self;
}
