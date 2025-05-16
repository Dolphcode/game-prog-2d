#include <math.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_input.h"
#include "gfc_color.h"

#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

#include "projectile.h"
#include "player.h"
#include "bug.h"
#include "camera.h"
#include "weapon.h"

#define GRAVITY 700

typedef struct {
	int	dashes;		// The number of dashes in the dash counter
	Sprite	*dash_counter;	// The sprite for the player's dash counter
	float	health_frac;	// The proportion of player health left
}PlayerHUD;

static PlayerHUD player_hud = {0};
static Entity *hook = NULL;
static Entity *player = NULL;

/**
 * @brief draws the player's hud
 */
void player_hud_draw() {
	for (int i = 0; i < player_hud.dashes; ++i) {
		GFC_Vector2D drawpos = gfc_vector2d(10 * (i + 1) + 32 * i, 52);
		gf2d_sprite_draw(player_hud.dash_counter,
				drawpos,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				0);
	}


	GFC_Rect healthbar = {0, 10, (int)(player_hud.health_frac * 400), 32};
	gf2d_draw_rect_filled(healthbar, GFC_COLOR_RED);
}

void player_hud_init() {
	player_hud.dash_counter = gf2d_sprite_load_all(
			"images/dash_count.png",
			32,
			32,
			1,
			0);
	atexit(player_hud_close);
}

void player_hud_close() {
	if (player_hud.dash_counter) gf2d_sprite_free(player_hud.dash_counter);
}

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
	GFC_Vector2D	lock_offset;	// <If grappled onto an entity, the offset vector
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
	
	Weapon	weapon;
}PlayerData;

/**
 * This function is called when the hook touches a static body
 */
void player_hook_static_touch(Entity *self) {
	if (!self) return;
	PlayerHookData *hook_data = (PlayerHookData*)self->data;
	if (!hook_data || hook_data->grappled || !hook_data->grapple_out) return;
	self->body->velocity = gfc_vector2d(0, 0);
	hook_data->grappled = 1;
	hook_data->grapple_length = gfc_vector2d_magnitude_between(self->body->position, player->body->position);
}

/**
 * This function is called every frame that the hook is touching a physics body
 */
void player_hook_touch(Entity *self, Entity *other) {
	if (!self) return;
	PlayerHookData *hook_data = (PlayerHookData*)self->data;
	if (!hook_data || hook_data->grappled) return;

	if (other->can_grapple && hook_data->grapple_out) {
		hook_data->lock_target = other;
		gfc_vector2d_sub(hook_data->lock_offset, self->position, other->position);
		self->body->velocity = gfc_vector2d(0, 0);
		hook_data->grappled = 1;
		hook_data->grapple_length = gfc_vector2d_magnitude_between(self->body->position, player->body->position);
	}
}

/**
 * Handle tracking entities we grappled onto
 */
void player_hook_think(Entity *self) {
	if (!self) return;
	PlayerHookData *data = (PlayerHookData*)self->data;
	if (!data) return;

	// Un grapple if the entity we are grappled to can no longer be grappled to
	if (data->grappled && data->lock_target) {
		if (!data->lock_target->can_grapple) {
			data->grapple_out = 0;
			data->grappled = 0;
			data->lock_target = NULL;
			gfc_vector2d_copy(self->position, data->player->position);
		} else {
			slog("target position %f %f", data->lock_target->position.x, data->lock_target->position.y); 
			gfc_vector2d_copy(self->position, data->lock_target->position);
		}
	}
}

/**
 * Handle drawing the hook itself
 */
void player_hook_draw(Entity *self) {

}

void player_touch(Entity *self, Entity *other) {

}

void player_think_ungrappled(Entity *self, PlayerData *player_data) {
	if (!self || !player_data) return;

	// Boosting
	if (player_data->boosting) {
		// Check if we have released and stop boosting at this point
		if (gfc_input_command_released("boost") || player_data->boost_timer <= 0) {
			player_data->boosting = 0;
		} else {
			self->velocity.x = player_data->boost_dir.x * player_data->boost_speed; // Maintain velocity?
			self->velocity.y = player_data->boost_dir.y * player_data->boost_speed; // Maintain velocity?
		}

		return; // Do not worry about any other movement stuff
	} else {
		if (gfc_input_command_pressed("boost") && player_data->dash_counter >= player_data->max_dashes) {
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
			if (player_data->boosting) {
			       	gfc_vector2d_normalize(&player_data->boost_dir);
				player_data->boost_timer = player_data->boost_time;
				player_data->dash_counter = 0;
			}
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
		if (gfc_input_command_released("boost") || player_data->boost_timer <= 0) {
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
		if (gfc_input_command_pressed("boost") && player_data->dash_counter >= player_data->max_dashes) {
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
			if (player_data->boosting) {
				gfc_vector2d_normalize(&player_data->boost_dir);
				player_data->boost_timer = player_data->boost_time;
				player_data->dash_counter = 0;
			}
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
	
	if (diff > 64) {
		slog("cutoff");
		hook_data->grapple_out = 0;
		hook_data->grappled = 0;
		hook_data->lock_target = NULL;
		gfc_vector2d_copy(hook->position, hook_data->player->position);
	} else if (diff > 0) {
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

	if (!self->alive) return;

	// Verify the hook pointer
	if (!player_data || !player_data->hook || !player_data->hook->data) return;
	PlayerHookData *hook_data = (PlayerHookData *)player_data->hook->data;
	
	// Firing and retracting the grappling hook
	if (gfc_input_command_pressed("hook_up") && hook) {
		if (!hook_data->grappled) {
			hook_data->grapple_out = 1;
			player_data->hook->velocity = gfc_vector2d(0, -200);
			gfc_vector2d_copy(hook->position, self->position);
		} else {
			hook_data->grapple_out = 0;
			hook_data->grappled = 0;
			hook_data->lock_target = NULL;
			gfc_vector2d_copy(hook->position, self->position);
		}
	}

	int mouse_down = SDL_GetMouseState(NULL, NULL);
	// Weapon firinig
	Weapon *wep = &player_data->weapon;
	if (mouse_down && wep->fire_time <= 0) {
		if (wep->has_ranged) {
			GFC_Vector2D offset, spawnpos;
			gfc_vector2d_copy(offset, wep->spawn_offset);
			offset = gfc_vector2d_rotate(offset, gfc_vector2d_angle(wep->direction));
			gfc_vector2d_add(spawnpos, offset, self->position);
			projectile_fire_ex(wep->projectile, spawnpos, wep->direction, wep->proj_speed, wep->spread, wep->inaccuracy, wep->count);
		}

		if (wep->has_melee) {
			wep->swinging = 1;
		}
		wep->fire_time = wep->rate;
	}

	if (!hook_data->grappled) {
		// Run ungrappled movement computations
		player_think_ungrappled(self, player_data);
	} else {
		// Run grappled movement computations
		player_think_grappled(self, hook, player_data, hook_data);
	}
	
}


void player_update(Entity *self) {
	if (!self) return;
	
	PlayerData *p_data = (PlayerData *)self->data;
	
	player_hud.dashes = p_data->dash_counter;
	
	if (p_data->dash_counter < p_data->max_dashes) {
		p_data->dash_cooldown_timer += 0.1;
		if (p_data->dash_cooldown_timer >= p_data->dash_cooldown) {
			p_data->dash_cooldown_timer = 0;
			p_data->dash_counter += 1;
		}
	}

	if (p_data->boost_timer > 0) {
		p_data->boost_timer -= 0.1;
	}

	if (self->i_time > 0) {
		self->i_time -= 0.1;
	} else {
		self->i_time = 0;
	}
	player_hud.health_frac = self->health / self->max_health;

      

	// Update weapon timer
	Weapon *wep = &p_data->weapon;
	if (wep->fire_time > 0) {
		wep->fire_time -= 0.1;

		if (wep->has_melee) {
			float rotation_interval = 0.1 * (wep->swing_angle / wep->rate);
			wep->rotation += rotation_interval;
			wep->direction = gfc_vector2d_rotate(wep->direction, rotation_interval / 180 * M_PI);
			GFC_Vector2D spawnpos, dir;
			gfc_vector2d_scale_by(dir, wep->direction, gfc_vector2d(16, 16));
			gfc_vector2d_copy(spawnpos, self->position);
			for (int i = 0; i < 10; ++i) {
				projectile_fire("sword_hit", spawnpos, gfc_vector2d(0, 0));
				gfc_vector2d_add(spawnpos, spawnpos, dir);
			}

		}

	} 
	if (wep->fire_time <= 0 && wep->has_melee && wep->swinging) {
		wep->swinging = 0;
	}	
	
	// Update weapon direction
	if (!p_data->weapon.has_melee || !p_data->weapon.swinging) {
	int mx, my;
	GFC_Vector2D mouse_dir, screen_res;
	SDL_GetMouseState(&mx, &my);
	mouse_dir.x = mx;
	mouse_dir.y = my;
	screen_res = gf2d_graphics_get_resolution();
	gfc_vector2d_scale_by(screen_res, screen_res, gfc_vector2d(0.5, 0.5));
	gfc_vector2d_sub(mouse_dir, mouse_dir, screen_res);
	gfc_vector2d_normalize(&mouse_dir);
	gfc_vector2d_copy(p_data->weapon.direction, mouse_dir); // Update direction of wepaon
	p_data->weapon.rotation = gfc_vector2d_angle(mouse_dir) * 180 / M_PI; // And its rotation value for drawing
	}	
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
	if (((int)self->i_time) % 2 == 0) entity_draw(self);

	GFC_Vector2D scale = main_camera_get_zoom();
	GFC_Vector2D draw_pos = main_camera_calc_drawpos(self->position);
	GFC_Vector2D center = data->weapon.sprite_offset;

	if (!data->weapon.has_melee || (data->weapon.has_melee && data->weapon.swinging)) {
		gf2d_sprite_draw(
			data->weapon.sprite,
			draw_pos,
			&scale,
			&center,
			&data->weapon.rotation,
			NULL,
			NULL,
			0);
	}
}

void player_damage(Entity *self, float amount) {
	if (!self || self->i_time > 0) return;
	self->i_time = self->immunity;
	self->health -= amount;
	if (self->health <= 0) self->alive = 0;
}

static char weapon[256];

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
	self->touch = player_touch;
	self->update = player_update;
	self->damage = player_damage;
	
	// Since this is a living thing, mark it as alive
	self->alive = 1;

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

	memset(&player_data->weapon, 0, sizeof(Weapon));
	weapon_load_from_file(&player_data->weapon, weapon);

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
	hook->touch = player_hook_touch;
	hook->think = player_hook_think;
	
	// Create the hook data
	PlayerHookData *hook_data = (PlayerHookData *)malloc(sizeof(PlayerHookData));
	if (!hook_data) {
		slog("failed to allocate memory for hook data");
		return NULL;
	}
	memset(hook_data, 0, sizeof(PlayerHookData));
	hook_data->player = self;
	hook->data = hook_data;
	player = self;

	player_data->hook = hook;
	return self;
}



Entity *player_spawn(GFC_Vector2D position, const char * config) {
	strcpy(weapon, config);
	Entity *player = player_new_entity(position);
	if (!player) return NULL;
	PlayerData *player_data = (PlayerData*)player->data;
	if (!player_data) return NULL;
	// Player weapon config
	return player;
}

// The cursor player stuff
#define CURSOR_SPEED 10
void cursor_player_think(Entity *self) {
	if (gfc_input_command_down("left")) {
		self->position.x -= CURSOR_SPEED;
	}
	if (gfc_input_command_down("right")) {
		self->position.x += CURSOR_SPEED;
	}
	if (gfc_input_command_down("up")) {
		self->position.y -= CURSOR_SPEED;
	}
	if (gfc_input_command_down("down")) {
		self->position.y += CURSOR_SPEED;
	}
}

Entity *cursor_player_spawn() {
	Entity *cursor = entity_new();
	if (!cursor) return NULL;
	cursor->think = cursor_player_think;
	return cursor;
}

