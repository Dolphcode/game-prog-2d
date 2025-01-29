#include "simple_logger.h"

#include "player.h"

Entity *player_new_entity(GFC_Vector2D position) {
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to spawn a new player entity");
		return NULL;
	}

	// Copy position data into player
	gfc_vector2d_copy(self->position,position);
	
	// Initialize and assign its sprite
	self->sprite = gf2d_sprite_load_all(
		"images/ed210.png",
		128,
		128,
		16,
		0);

	return self;
}
