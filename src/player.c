#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_input.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

#include "player.h"
#include "bug.h"
#include "camera.h"

static float projv1 = 2;
static float projv2 = 1;

void player_update(Entity *self) {
	if (!self) return;
	float ogy=self->velocity.y;	
	
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
	gfc_vector2d_scale_by(self->velocity, self->velocity, gfc_vector2d(100, 100));
	self->velocity.y += ogy;
	self->acceleration.y = 9.8;

	//gfc_vector2d_add(self->position, self->position, self->velocity);
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
	self->think = player_update;

	return self;
}
