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
	// Verify pointers
	if (!self) return;
	if (!self->sprite) return;

	// Calculate drawpos, center, and scale
	GFC_Vector2D drawpos = {0};
	gfc_vector2d_sub(drawpos, self->position, camera_get_main()->position);
	gfc_vector2d_sub(drawpos, drawpos, gfc_vector2d(self->sprite->frame_w / 2.0, self->sprite->frame_h / 2.0));
	gfc_vector2d_scale_by(drawpos, drawpos, gfc_vector2d(camera_get_main()->zoom, camera_get_main()->zoom));
	GFC_Vector2D screen_res = gf2d_graphics_get_resolution();
	screen_res.x /= 2.0;
	screen_res.y /= 2.0;
	gfc_vector2d_add(drawpos, drawpos, screen_res);
	
	GFC_Vector2D scale = gfc_vector2d(camera_get_main()->zoom, camera_get_main()->zoom);
	GFC_Vector2D center = {0};
	gfc_vector2d_sub(center, drawpos, screen_res);
	gfc_vector2d_negate(center, center);
	//GFC_Vector2D center = gfc_vector2d(self->sprite->frame_w / 2.0, self->sprite->frame_h / 2.0);
	//gfc_vector2d_scale_by(center, gf2d_graphics_get_resolution(), gfc_vector2d(0.5, 0.5));

	// Draw the player sprite
	gf2d_sprite_draw(
		self->sprite,
		drawpos,
		&scale,
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

	return self;
}
