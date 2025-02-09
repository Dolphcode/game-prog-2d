#include "simple_logger.h"

#include "gf2d_sprite.h"

#include "player.h"

void player_think(Entity *self) {
	if (!self) return;
}

void player_draw(Entity *self) {
	// Draw the player sprite
	if (!self) return;
	if (!self->sprite) return;
	gf2d_sprite_draw(
		self->sprite,
		self->position,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		(Uint32)self->frame);

	// Update the frame
	self->frame += 0.1;
	if (self->frame >= 16.0) {
		self->frame = 0.0;
	}
}

Entity *player_new_entity(GFC_Vector2D position) {
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to spawn a new player entity");
		return NULL;
	}

	// Copy position date into player
	gfc_vector2d_copy(self->position, position);

	// Initialize and assign sprite
	self->sprite = gf2d_sprite_load_all(
			"images/ed210.png",
			128,
			128,
			16,
			0);
	
	// Assign player functions
	self->think = player_think;
	self->draw = player_draw;

	return self;
}
