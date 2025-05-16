#include "simple_logger.h"
#include "simple_json.h"

#include "gf2d_draw.h"
#include "gf2d_sprite.h"
#include "gf2d_graphics.h"

#include "world.h"
#include "projectile.h"
#include "firing_enemy.h"

typedef struct {
	const char	projectile[256];// <What projectile this guy fires
	float		rate;		// <The fire rate of the firing enemy
	int		count;		// <How many projectils this guy should shoot
	float		spread;		// <The angle spread of the projectiles fired
	float		inaccuracy;	// <Inaccuracy (random range for firing) of projectiles
	float		hover_distance;	// <Distance that this guy will hover from the player
	float		timer;		// <How long before next fire
	float		speed;		// <The speed of the fired projectiles 
	float		hover_speed;	// <The hover speed of the enemy
	float		accel;		// <
	float		decel;		// <
	float		max_speed;	// <
}FiringEnemyData;

void firing_enemy_think(Entity *self) {
	if (!self || !self->alive || !world_get_active() || !world_get_active()->player) return;
	FiringEnemyData *data = (FiringEnemyData *)self->data;
	if (!data) return;

	// Get the player reference
	Entity *player_ref = world_get_active()->player;

	GFC_Vector2D direction, fire_dir;
	if (gfc_vector2d_magnitude_between(player_ref->position, self->position) > data->hover_distance) {
		if (gfc_vector2d_magnitude(self->velocity) < data->max_speed) {
			gfc_vector2d_sub(direction, player_ref->position, self->position);
			gfc_vector2d_normalize(&direction);
			gfc_vector2d_scale_by(direction, direction, gfc_vector2d(data->accel, data->accel));
			gfc_vector2d_add(self->acceleration, self->acceleration, direction);
		} else {
			gfc_vector2d_set_magnitude(&self->velocity, data->max_speed);
		}
	} else {
		gfc_vector2d_negate(direction, self->velocity);
		gfc_vector2d_normalize(&direction);
		if (gfc_vector2d_magnitude(self->velocity) > data->hover_speed) {
			gfc_vector2d_scale_by(direction, direction, gfc_vector2d(data->decel, data->decel));
			gfc_vector2d_add(self->acceleration, self->acceleration, direction);
		} else {
			//self->velocity.x = 0;
			//self->velocity.y = 0;
		}
	}

	if (data->timer >= data->rate) {
		data->timer = 0;
		gfc_vector2d_sub(fire_dir, player_ref->position, self->position);
		gfc_vector2d_normalize(&fire_dir);
		projectile_fire_ex(data->projectile, self->position, fire_dir, data->speed, data->spread, data->inaccuracy, data->count);
	}
}

void firing_enemy_update(Entity *self) {
	if (!self || !self->alive || !world_get_active() || !world_get_active()->player) return;
	FiringEnemyData *data = (FiringEnemyData *)self->data;
	if (!data) return;

	data->timer += 0.1;

	if (self->i_time > 0) {
		self->i_time -= 0.1;
	} else {
		self->i_time = 0;
	}
}

void firing_enemy_draw(Entity *self) {
	if (!self || !self->alive || !world_get_active() || !world_get_active()->player) return;
	FiringEnemyData *data = (FiringEnemyData *)self->data;
	if (!data) return;

	// Get the player reference
	Entity *player_ref = world_get_active()->player;

	GFC_Vector2D scale = main_camera_get_zoom();

	GFC_Vector2D draw_pos = main_camera_calc_drawpos(self->position);

	GFC_Vector2D center = self->sprite_offset;

	GFC_Vector2D face;
	gfc_vector2d_sub(face, player_ref->position, self->position);
	float rot = gfc_vector2d_angle(face) * 180 / M_PI + 180;


	// Draw the sprite
	gf2d_sprite_draw(
		self->sprite,
		draw_pos,
		&scale,
		&center,
		&rot,
		NULL,
		NULL,
		(Uint32)self->frame);

	// Draw the point
	if (DRAW_CENTER) gf2d_draw_circle(draw_pos, 4, GFC_COLOR_LIGHTGREEN);
}

void firing_enemy_damage(Entity *self, float damage) {
	if (!self || self->i_time > 0) return;
	self->i_time = self->immunity;
	self->health -= damage;
	if (self->health <= 0) {
		self->alive = 0;
		self->body->disabled = 1;
	}
}

Entity *firing_enemy_spawn(GFC_Vector2D position, const char *config) {
	// INITIAL ENTITY INITIALIZATION
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to spawn a new player entity");
		return NULL;
	}

	// Copy position date into entity
	gfc_vector2d_copy(self->position, position);

	// Get the player config file and configure entity from the file
	// We'll be reusing the player config file later
	SJson *json = sj_load(config);
	if (!json) {
		slog("failed to open def file");
		return NULL;
	}
	entity_configure(self, json);

	// Assign player functions
	self->think = firing_enemy_think;
	self->update = firing_enemy_update;
	self->draw = firing_enemy_draw;
	self->damage = firing_enemy_damage;

	// Since this is a living thing, mark it as alive
	self->alive = 1;

	// Create the player data object
	FiringEnemyData *data = (FiringEnemyData*)malloc(sizeof(FiringEnemyData));
	if (!data) {
		slog("failed to allocate memory for data");
		return NULL;
	}
	
	memset(data, 0, sizeof(FiringEnemyData));

	SJson *data_json = sj_object_get_value(json, "firingEnemy");
	if (!data_json) {
		slog("%s is missing a 'firingEnemy' object", config);
		return NULL;
	}
	const char *proj_name = sj_object_get_string(data_json, "projectile");
	strcpy(data->projectile, proj_name);
	sj_object_get_float(data_json, "rate", &data->rate);
	sj_object_get_int(data_json, "count", &data->count);
	sj_object_get_float(data_json, "spread", &data->spread);
	sj_object_get_float(data_json, "inaccuracy", &data->inaccuracy);
	sj_object_get_float(data_json, "hoverDistance", &data->hover_distance);
	sj_object_get_float(data_json, "accel", &data->accel);
	sj_object_get_float(data_json, "decel", &data->decel);
	sj_object_get_float(data_json, "maxSpeed", &data->max_speed);
	sj_object_get_float(data_json, "speed", &data->speed);
	sj_object_get_float(data_json, "hoverSpeed", &data->hover_speed);
	self->data = data;

	sj_free(json);

	return self;
}
