#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_input.h"

#include "gf2d_sprite.h"

#include "player.h"
#include "bug.h"

static float projv1 = 5;
static float projv2 = 3;

void player_update(Entity *self) {
	if (!self) return;
	
	// zero velocity
	self->velocity = gfc_vector2d(0, 0);
	
	// check input
	if (gfc_input_command_down("left")) {
		self->velocity.x -= 1;
	}
	if (gfc_input_command_down("right")) {
		self->velocity.x += 1;
	}
	if (gfc_input_command_down("up")) {
		self->velocity.y -= 1;
	}
	if (gfc_input_command_down("down")) {
		self->velocity.y += 1;
	}

	if (gfc_input_command_pressed("shoot1")) {
		Entity *bug = bug_new_entity(self->position, "./def/bugs/bug1.def");
		bug->velocity = gfc_vector2d(projv1, 0);
	}

	if (gfc_input_command_pressed("shoot2")) {
		Entity *bug = bug_new_entity(self->position, "./def/bugs/bug2.def");
		bug->velocity = gfc_vector2d(0, projv2);
	}
	
	gfc_vector2d_normalize(&self->velocity);
	gfc_vector2d_scale_by(self->velocity, self->velocity, gfc_vector2d(5, 5));

	gfc_vector2d_add(self->position, self->position, self->velocity);
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

	// Initialize player entity from config
	entity_configure_from_file(self, "./def/player.def");
	
	// Assign player functions
	self->update = player_update;
	self->draw = player_draw;

	return self;
}
