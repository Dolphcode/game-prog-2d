#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_input.h"
#include "gfc_color.h"

#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

#include "player.h"
#include "bug.h"
#include "camera.h"

static float projv1 = 2;
static float projv2 = 1;

static Uint8 grappled = 0;
static float grapple_length = 0;
static Entity *hook = NULL;

static Entity *player = NULL;


void player_hook_static_touch(Entity *self) {
	if (!self || grappled) return;
	slog("touched");
	self->body->velocity = gfc_vector2d(0, 0);
	grappled = 1;
	grapple_length = gfc_vector2d_magnitude_between(self->body->position, player->body->position);
}

void player_update(Entity *self) {
	if (!self) return;
	//float ogy=self->velocity.y;	
	
	// zero velocity
	//self->velocity = gfc_vector2d(0, 0);
	
	// check input
	if (gfc_input_command_down("dash")) {
		if (gfc_input_command_pressed("left")) {
			self->velocity.x -= 800;
		}
		if (gfc_input_command_pressed("right")) {
			self->velocity.x += 800;
		}

		if (gfc_input_command_pressed("up")) {
			self->velocity.y -= 800;
		}
		if (gfc_input_command_pressed("down")) {
			self->velocity.y += 800;
		}
	} else if (self->body->grounded) {
		
		float net_vel = 0;
		if (gfc_input_command_down("left")) {
			net_vel += -40;
		}
		if (gfc_input_command_down("right")) {
			net_vel += 40;
		}
		self->velocity.x = net_vel;
	}

	if (gfc_input_command_pressed("hook_up") && hook) {
		if (!grappled) {
			hook->velocity = gfc_vector2d(0, -200);
			gfc_vector2d_copy(hook->position, self->position);
		} else {
			grappled = !grappled;
			gfc_vector2d_copy(hook->position, self->position);
		}
	}

	if (gfc_input_command_pressed("shoot1")) {
		Entity *bug = bug_new_entity(self->position, "./def/bugs/bug1.def");
		bug->velocity = gfc_vector2d(projv1, 0);
	}

	if (gfc_input_command_pressed("shoot2")) {
		Entity *bug = bug_new_entity(self->position, "./def/bugs/bug2.def");
		bug->velocity = gfc_vector2d(0, projv2);
	}

	if (gfc_input_command_down("retract") && grappled && grapple_length > 10) {
		grapple_length -= 10;
	}

	self->acceleration.y += 500;
	
	if (gfc_vector2d_magnitude(self->velocity) > 500) {
		float mag = gfc_vector2d_magnitude(self->velocity);
		GFC_Vector2D dir;
		gfc_vector2d_negate(dir, self->velocity);
		gfc_vector2d_normalize(&dir);
		gfc_vector2d_scale_by(dir, dir, gfc_vector2d(mag * 1, mag * 1));
		// Drag constant
		gfc_vector2d_add(self->acceleration, self->acceleration, dir);

	}
	if (gfc_vector2d_magnitude(self->velocity) < 500) {
		float mag = gfc_vector2d_magnitude(self->velocity);
		GFC_Vector2D dir;
		gfc_vector2d_negate(dir, self->velocity);
		gfc_vector2d_normalize(&dir);
		gfc_vector2d_scale_by(dir, dir, gfc_vector2d(mag * 0.1, mag * 0.1));
		// Drag constant
		gfc_vector2d_add(self->acceleration, self->acceleration, dir);


	}
	if (grappled) {
		GFC_Vector2D dir;
		float magbetw = gfc_vector2d_magnitude_between(self->position, hook->position);
		float diff = magbetw - grapple_length;
		float speed = gfc_vector2d_magnitude(self->velocity);
		gfc_vector2d_sub(dir, hook->position, self->position);
		gfc_vector2d_normalize(&dir);

		if (diff > 0) {
			GFC_Vector2D correction;
			gfc_vector2d_scale_by(correction, dir, gfc_vector2d(diff, diff));

			gfc_vector2d_add(self->position, self->position, correction);

			GFC_Vector2D velocity_corr;
			float comp = gfc_vector2d_dot_product(self->velocity, dir);
			if (comp < 0) {
				gfc_vector2d_scale_by(velocity_corr, dir, gfc_vector2d(-comp, -comp));
			} else {
				gfc_vector2d_scale_by(velocity_corr, dir, gfc_vector2d(comp, comp));
			}
			gfc_vector2d_add(self->velocity, self->velocity, velocity_corr);
		}

			//	gfc_vector2d_scale_by(dir, dir, gfc_vector2d(diff * 100, diff * 100));
		//gfc_vector2d_add(self->acceleration, self->acceleration, dir);
	}
}

void player_draw(Entity *self) {
	if (!self) return;
	GFC_Vector2D player_point = main_camera_calc_drawpos(self->position);
	GFC_Vector2D hook_point = main_camera_calc_drawpos(hook->position);
	if (grappled) {
		gf2d_draw_line(player_point, hook_point, GFC_COLOR_BLACK);
	}
	entity_draw(self);
}

void player_static_touch(Entity *self) {
	slog("thunk");
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
	self->static_touch = player_static_touch;
	self->draw = player_draw;

	// Now create the grappling hook
	hook = entity_new();
	if (!hook) {
		slog("failed to spawn a new grappling hook");
		return NULL;
	}

	gfc_vector2d_copy(hook->position, self->position);
	entity_configure_from_file(hook, "./def/player_hook.def");
	hook->static_touch = player_hook_static_touch;
	player = self;
	return self;
}
