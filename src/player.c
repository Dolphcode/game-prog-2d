#include <math.h>

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

#define GRAVITY 700

static float projv1 = 2;
static float projv2 = 1;

static Uint8 grappled = 0;
static float grapple_length = 0;
static Entity *hook = NULL;

static Entity *player = NULL;
static Uint8 boosting = 0;
static GFC_Vector2D boost_dir;

/**
 * Represents information that is unique to the player's grappling hook
 */
typedef struct {
	Uint8	grappled;	// Whether the hook is attached to something
	Uint8	grapple_out;	// Whether the hook is out or not
	float	grapple_length;	// The length of the hook;
	float	max_grapple;	// The maximum length of the grappling hook
	
	Entity	*player;	// The player reference (where the hook is being fired from)
	Entity	*lock_target;	// The target which this hook is locked to
}PlayerHookData;

/**
 * Represents information that is unique to the player
 */
typedef struct {
	Uint8	boosting; 		// Whether the player is currently boosting or not
	float	boost_time;		// How long a boost can last
	float	boost_timer;		// How long before the boost ends
	float	boost_speed;		// The speed of a boost
	GFC_Vector2D	boost_dir;	// The direction of the boost

	Uint8	max_dashes;		// The maximum number of dashes a player can have
	Uint8	dash_counter;		// Number of dashes the player has available
	float	dash_speed;
	float	dash_cooldown;		// Total cooldown time for dash
	float	dash_cooldown_timer;	// Time to next dash refill
				
	float	grounded_max_speed;	// The player's maximum speed while grounded
	float	air_max_speed;		// The player's maximum speed while in the air
	float	restorative_accel;	// The scaling factor for the restorative acceleration when accelerating over max velocity

	float	ground_accel;		// The grounded acceleration value of the player
	
	Entity	*hook;			// A reference to the player's grappling hook
}PlayerData;

/**
 * This function is called when the hook touches a static body
 */
void player_hook_static_touch(Entity *self) {
	if (!self) return;
	PlayerHookData *hook_data = (PlayerHookData*)self->data;
	if (!hook_data || hook_data->grappled) return;
	self->body->velocity = gfc_vector2d(0, 0);
	hook_data->grappled = 1;
	hook_data->grapple_length = gfc_vector2d_magnitude_between(self->body->position, player->body->position);
}

/**
 * This function is called every frame that the hook is touching a physics body
 */
void player_hook_touch(Entity *self, Entity *other) {

}

/**
 * Handle tracking entities we grappled onto
 */
void player_hook_think(Entity *self) {

}

/**
 * Handle drawing the hook itself
 */
void player_hook_draw(Entity *self) {

}

void player_think_ungrappled(Entity *self, PlayerData *player_data) {
	if (!self || !player_data) return;

	// Boosting
	if (player_data->boosting) {
		// Check if we have released and stop boosting at this point
		if (gfc_input_command_released("boost")) {
			player_data->boosting = 0;
		} else {
			self->velocity.x = player_data->boost_dir.x * player_data->boost_speed; // Maintain velocity?
			self->velocity.y = player_data->boost_dir.y * player_data->boost_speed; // Maintain velocity?
		}

		return; // Do not worry about any other movement stuff
	} else {
		if (gfc_input_command_pressed("boost")) {
			// Reset
			player_data->boost_dir.x = 0;
			player_data->boost_dir.y = 0;

			if (gfc_input_command_down("left")) {
				player_data->boost_dir.x -= 1;
				player_data->boosting = 1;
			}
			if (gfc_input_command_down("right")) {
				player_data->boost_dir.x += 1;
				player_data->boosting = 1;
			}
			if (gfc_input_command_down("up")) {
				player_data->boost_dir.y -= 1;
				player_data->boosting = 1;
			}
			if (gfc_input_command_down("down")) {
				player_data->boost_dir.y += 1;
				player_data->boosting = 1;
			}
			
			// Normalize the direction vector if possible
			if (player_data->boosting) gfc_vector2d_normalize(&player_data->boost_dir);
		}
	}

	// Dashing
	if (gfc_input_command_down("dash") && player_data->dash_counter) {
		if (gfc_input_command_pressed("left")) {
			player_data->dash_counter--;
			self->velocity.x = -800;
		}

		if (gfc_input_command_pressed("right")) {
			player_data->dash_counter--;
			self->velocity.x = 800;
		}

		if (gfc_input_command_pressed("up")) {
			player_data->dash_counter--;
			self->velocity.y = -800;
		}

		if (gfc_input_command_pressed("down")) {
			player_data->dash_counter--;
			self->velocity.y = 800;
		}
	}

	// Check if the player is grounded or not
	// This will determine the drag force computation and what happens when pressing WASD
	if (self->body->grounded) {
		// Apply a motion force
		float net_dir = 0;
		Uint8 input = 0;
		if (gfc_input_command_down("left")) {
			net_dir -= 1;
			input = 1;
		}
		if (gfc_input_command_down("right")) {
			net_dir += 1;
			input = 1;
		}	
		
		// Check the current speed
		float curr_speed = gfc_vector2d_magnitude(self->velocity);
		float max_speed = (input) ? player_data->grounded_max_speed : 0;

		// Apply a restorative force if we are over the maximum grounded speed
		if (curr_speed > max_speed) {
			if (curr_speed - max_speed < 20) {
				gfc_vector2d_set_magnitude(&self->velocity, max_speed);	
			} else {
				// Determine the direction of the drag force
				GFC_Vector2D restore_direction;
				gfc_vector2d_negate(restore_direction, self->velocity);
				gfc_vector2d_normalize(&restore_direction);
	
				// Multiply it by the over speed
				GFC_Vector2D restore_force, scale_factor;
				scale_factor = gfc_vector2d(player_data->restorative_accel, player_data->restorative_accel);
				gfc_vector2d_scale_by(restore_force, restore_direction, scale_factor);
	
				// Apply the restorative force
				self->acceleration.x += restore_force.x;
				self->acceleration.y += restore_force.y;
			}
		} else {
			GFC_Vector2D move_acceleration = gfc_vector2d(net_dir * player_data->ground_accel, 0); 
			gfc_vector2d_add(self->acceleration, self->acceleration, move_acceleration);
		}

	} else {
		// Check the current speed
		float curr_speed = gfc_vector2d_magnitude(self->velocity);

		// Apply a restorative force if we are over the maximum air speed
		if (curr_speed > player_data->air_max_speed) {	
			if (curr_speed - player_data->air_max_speed < 20) {
				gfc_vector2d_set_magnitude(&self->velocity, player_data->air_max_speed);	
			} else {
				// Determine the direction of the drag force
				GFC_Vector2D restore_direction;
				gfc_vector2d_negate(restore_direction, self->velocity);
				gfc_vector2d_normalize(&restore_direction);
	
				// Multiply it by the over speed
				GFC_Vector2D restore_force, scale_factor;
				scale_factor = gfc_vector2d(player_data->restorative_accel, player_data->restorative_accel);
				gfc_vector2d_scale_by(restore_force, restore_direction, scale_factor);
	
				// Apply the restorative force
				gfc_vector2d_add(self->acceleration, self->acceleration, restore_force);
			}
		}

		// Apply gravity
		self->acceleration.y += GRAVITY;

	}

}

void player_think_grappled(Entity *self, Entity *hook, PlayerData *player_data, PlayerHookData *hook_data) {
	if (!self || !hook || !player_data || !hook_data) return;

	// Compute secant and tangent vectors
	GFC_Vector2D secant, tangent; // Secant is towards the center, Tangent is CW
	gfc_vector2d_sub(secant, hook->position, self->position);
	gfc_vector2d_normalize(&secant);
	tangent = gfc_vector2d_rotate(secant, 0.5 * M_PI);

	// Boosting
	if (player_data->boosting) {
		// Check if we have released and stop boosting at this point
		if (gfc_input_command_released("boost")) {
			player_data->boosting = 0;
		} else {
			if (player_data->boost_dir.x > 0) {
				gfc_vector2d_scale_by(self->velocity, tangent, gfc_vector2d(player_data->boost_speed, player_data->boost_speed));
			} else if (player_data->boost_dir.x < 0) {
				gfc_vector2d_scale_by(self->velocity, tangent, gfc_vector2d(-player_data->boost_speed, -player_data->boost_speed));
			} else if (player_data->boost_dir.y > 0) {			
				self->velocity.x = 0; // Maintain velocity?
				self->velocity.y = player_data->boost_dir.y * player_data->boost_speed; // Maintain velocity?
			} else if (player_data->boost_dir.y < 0) {
				self->velocity.x = 0; // Maintain velocity?
				self->velocity.y = player_data->boost_dir.y * player_data->boost_speed; // Maintain velocity?
			}

		}

	} else {
		if (gfc_input_command_pressed("boost")) {
			// Reset
			player_data->boost_dir.x = 0;
			player_data->boost_dir.y = 0;

			if (gfc_input_command_down("left")) {
				player_data->boost_dir.x -= 1;
				player_data->boosting = 1;
			}
			if (gfc_input_command_down("right")) {
				player_data->boost_dir.x += 1;
				player_data->boosting = 1;
			}
			if (gfc_input_command_down("up")) {
				player_data->boost_dir.y -= 1;
				player_data->boosting = 1;
			}
			if (gfc_input_command_down("down")) {
				player_data->boost_dir.y += 1;
				player_data->boosting = 1;
			}
			
			// Normalize the direction vector if possible
			if (player_data->boosting) gfc_vector2d_normalize(&player_data->boost_dir);
		}
	}		
	
	// Dashing
	GFC_Vector2D dash_impulse;
	if (gfc_input_command_down("dash") && player_data->dash_counter && !player_data->boosting) {
		if (gfc_input_command_pressed("left")) {
			player_data->dash_counter--;
			gfc_vector2d_scale_by(dash_impulse, tangent, gfc_vector2d(-800, -800));
			gfc_vector2d_add(self->velocity, self->velocity, dash_impulse);
		}

		if (gfc_input_command_pressed("right")) {
			player_data->dash_counter--;
			gfc_vector2d_scale_by(dash_impulse, tangent, gfc_vector2d(800, 800));
			gfc_vector2d_add(self->velocity, self->velocity, dash_impulse);
		}

		if (gfc_input_command_pressed("up")) {
			player_data->dash_counter--;
			self->velocity.y = -800;
		}

		if (gfc_input_command_pressed("down")) {
			player_data->dash_counter--;
			self->velocity.y = 800;
		}
	}

	// ------------------------------------------------------------- MAY OMIT ---------------------------------------
	// Apply an aerial restorative force
	// Check the current speed
	float curr_speed = gfc_vector2d_magnitude(self->velocity);
	// Apply a restorative force if we are over the maximum air speed
	if (curr_speed > player_data->air_max_speed) {	
		if (curr_speed - player_data->air_max_speed < 20) {
			gfc_vector2d_set_magnitude(&self->velocity, player_data->air_max_speed);	
		} else {
			// Determine the direction of the drag force
			GFC_Vector2D restore_direction;
			gfc_vector2d_negate(restore_direction, self->velocity);
			gfc_vector2d_normalize(&restore_direction);
	
			// Multiply it by the over speed
			GFC_Vector2D restore_force, scale_factor;
			scale_factor = gfc_vector2d(player_data->restorative_accel, player_data->restorative_accel);
			gfc_vector2d_scale_by(restore_force, restore_direction, scale_factor);
	
			// Apply the restorative force
			gfc_vector2d_add(self->acceleration, self->acceleration, restore_force);
		}
	}
	// ------------------------------------------------------------- MAY OMIT ---------------------------------------

	// Hook Correction
	GFC_Vector2D dir;
	float magbetw = gfc_vector2d_magnitude_between(self->position, hook->position);
	float diff = magbetw - hook_data->grapple_length;
	gfc_vector2d_sub(dir, hook->position, self->position);
	gfc_vector2d_normalize(&dir);

	slog("%f magbetw %f diff %f", hook_data->grapple_length, magbetw, diff);
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

	// Retract the hook
	if (gfc_input_command_down("retract") && hook_data->grapple_length > 10) {
		hook_data->grapple_length -= 10;
	}

	// Do not apply gravity if boosting
	if (!player_data->boosting) {
		self->acceleration.y += GRAVITY;
	}
}

void player_think(Entity *self) {
	// Verify the player pointer
	if (!self || !self->data) return;
	PlayerData *player_data = (PlayerData *)self->data;

	// Verify the hook pointer
	if (!player_data || !player_data->hook || !player_data->hook->data) return;
	PlayerHookData *hook_data = (PlayerHookData *)player_data->hook->data;
	
	// Firing and retracting the grappling hook
	if (gfc_input_command_pressed("hook_up") && hook) {
		if (!hook_data->grappled) {
			player_data->hook->velocity = gfc_vector2d(0, -200);
			gfc_vector2d_copy(hook->position, self->position);
		} else {
			hook_data->grappled = 0;
			gfc_vector2d_copy(hook->position, self->position);
		}
	}

	if (!hook_data->grappled) {
		// Run ungrappled movement computations
		player_think_ungrappled(self, player_data);
	} else {
		// Run grappled movement computations
		player_think_grappled(self, hook, player_data, hook_data);
	}
	
}



/*
 * boost command - "boost"
 * "left"
 * "right"
 * "up"
 * "down"
 * "dash"
 * "hook_up"
 * "retract"
 */
void player_update(Entity *self) {
	if (!self) return;
	
	PlayerData *player_data = (PlayerData *)self->data;
	if (!player_data) return;
	

	if (boosting) {
		if (gfc_input_command_released("boost")) boosting = 0;
		gfc_vector2d_scale_by(self->velocity, boost_dir, gfc_vector2d(600, 600));
		return;
	}

	if (gfc_input_command_pressed("boost")) {
		if (gfc_input_command_down("left")) {
			boost_dir.x = -1;
			boosting = 1;
		} else if (gfc_input_command_down("right")) {
			boost_dir.x = 1;
			boosting = 1;
		} else {
			boost_dir.x = 0;
		}

		if (gfc_input_command_down("up")) {
			boost_dir.y = -1;
			boosting = 1;
		} else if (gfc_input_command_down("down")) {
			boost_dir.y = 1;
			boosting = 1;
		} else {
			boost_dir.y = 0;
		}

		if (boosting) {
			gfc_vector2d_normalize(&boost_dir);
			gfc_vector2d_scale_by(self->velocity, boost_dir, gfc_vector2d(player_data->boost_speed, player_data->boost_speed));
		}
	}

	// check input
	if (gfc_input_command_down("dash")) {
		if (gfc_input_command_pressed("left")) {
			self->velocity.x -= player_data->dash_speed;
		}
		if (gfc_input_command_pressed("right")) {
			self->velocity.x += player_data->dash_speed;
		}

		if (gfc_input_command_pressed("up")) {
			self->velocity.y -= player_data->dash_speed;
		}
		if (gfc_input_command_pressed("down")) {
			self->velocity.y += player_data->dash_speed;
		}
	}
	
	if (self->body->grounded) {
		
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

	// Determine what the player's max speed should be
	float max_speed;
	if (self->body->grounded) {
		max_speed = player_data->grounded_max_speed;
	} else {
		max_speed = player_data->air_max_speed;
	}

	// If we are above the max speed apply a restorative force
}

void player_draw(Entity *self) {
	if (!self) return;
	PlayerData *data = (PlayerData*)self->data;
	if (!data) return;
	Entity *hook = data->hook;
	if (!hook) return;
	PlayerHookData *hook_data = (PlayerHookData*)hook->data;
	if (!hook_data) return;

	GFC_Vector2D player_point = main_camera_calc_drawpos(self->position);
	GFC_Vector2D hook_point = main_camera_calc_drawpos(hook->position);

	// Draw the rope of the hook
	if (hook_data->grappled) {
		gf2d_draw_line(player_point, hook_point, GFC_COLOR_BLACK);
	}
	entity_draw(self);
}


Entity *player_new_entity(GFC_Vector2D position) {
	// INITIAL ENTITY INITIALIZATION
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to spawn a new player entity");
		return NULL;
	}

	// Copy position date into player
	gfc_vector2d_copy(self->position, position);

	// Get the player config file and configure entity from the file
	// We'll be reusing the player config file later
	SJson *json = sj_load("./def/player.def");
	if (!json) {
		slog("failed to open def file");
		return NULL;
	}
	entity_configure(self, json);
	
	// Assign player functions
	self->think = player_think;
	self->draw = player_draw;


	// Create the player data object
	PlayerData *player_data = (PlayerData*)malloc(sizeof(PlayerData));
	if (!player_data) {
		slog("failed to allocate memory for player data");
		return NULL;
	}
	
	memset(player_data, 0, sizeof(PlayerData));
	SJson *data_json = sj_object_get_value(json, "player");
	if (!data_json) {
		slog("player.def is missing a 'player' object");
		return NULL;
	}

	sj_object_get_uint8(data_json, "maxDashes", &player_data->max_dashes);
	sj_object_get_float(data_json, "maxAirSpeed", &player_data->air_max_speed);
	sj_object_get_float(data_json, "maxGroundSpeed", &player_data->grounded_max_speed);
	sj_object_get_float(data_json, "boostTime", &player_data->boost_time);
	sj_object_get_float(data_json, "dashCooldown", &player_data->dash_cooldown);
	sj_object_get_float(data_json, "dashSpeed", &player_data->dash_speed);
	sj_object_get_float(data_json, "boostSpeed", &player_data->boost_speed);
	sj_object_get_float(data_json, "restorativeAccel", &player_data->restorative_accel);
	sj_object_get_float(data_json, "groundAccel", &player_data->ground_accel);
	player_data->dash_counter = player_data->max_dashes;
	self->data = player_data; // Assign the player data object

	// GRAPPLING HOOK INITIALIZATION

	// Now create the grappling hook
	hook = entity_new();
	if (!hook) {
		slog("failed to spawn a new grappling hook");
		return NULL;
	}
	
	// Spawn the hook
	gfc_vector2d_copy(hook->position, self->position);
	entity_configure_from_file(hook, "./def/player_hook.def");
	hook->static_touch = player_hook_static_touch;
	
	// Create the hook data
	PlayerHookData *hook_data = (PlayerHookData *)malloc(sizeof(PlayerHookData));
	if (!hook_data) {
		slog("failed to allocate memory for hook data");
		return NULL;
	}
	memset(hook_data, 0, sizeof(PlayerHookData));
	hook_data->player = self;
	hook->data = hook_data;

	player_data->hook = hook;
	player = self;
	return self;
}
