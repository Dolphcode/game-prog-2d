#include "gfc_input.h"
#include "gfc_vector.h"

#include "simple_logger.h"

#include "gf2d_graphics.h"

#include "player.h"

void player_think(Entity* self) {
	if (!self) return;
	
	// Reset velocity
	self->velocity.x = 0;
	self->velocity.y = 0;
	
	// Calculate velocity
	if (gfc_input_command_down("playerUp")) {
		self->velocity.y -= 2;
	}
	if (gfc_input_command_down("playerDown")) {
		self->velocity.y += 2;
	}
	if (gfc_input_command_down("playerLeft")) {
		self->velocity.x -= 2;
	}
	if (gfc_input_command_down("playerRight")) {
		self->velocity.x += 2;
	}
	
	// Apply velocity
	gfc_vector2d_add(self->position, self->position, self->velocity);
	
	// Check bounds
	
	GFC_Vector2D res = gf2d_graphics_get_resolution();
	if (self->position.x < -80) self->position.x = res.x;
	else if (self->position.x > res.x) self->position.x = -80;

	if (self->position.y < -80) self->position.y = res.y;
	else if (self->position.y > res.y) self->position.y = -80;
	
}

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

	// Assign the think function
	self->think = player_think;

	return self;
}
