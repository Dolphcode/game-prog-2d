#include "simple_logger.h"

#include "gf2d_sprite.h"

#include "bug.h"

void bug_draw(Entity *self) {
	if (!self || !self->sprite) return;
	gf2d_sprite_draw(
		self->sprite,
		self->position,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		(Uint32)self->frame);
}

void bug_think(Entity *self) {

}

void bug_update(Entity *self) {
	if (!self) return;

	self->lifetime += 0.1;
	if (self->lifetime > 15) {
		entity_free(self);
		return;
	}

	self->position.x += 2;
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

	// Assign functions
	self->draw = bug_draw;
	self->think = bug_think;
	self->update = bug_update;

	return self;
}
