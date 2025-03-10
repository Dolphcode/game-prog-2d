#include "simple_logger.h"
#include "simple_json.h"

#include "gf2d_draw.h"
#include "gf2d_sprite.h"
#include "gf2d_graphics.h"

#include "world.h"
#include "ram_enemy.h"

typedef struct {
	float	ram_time;	// <The amount of time between each dash
	float	timer;		// <The timer that counts to the next dash
	float	dash_speed;	// <The dash speed/impulse applied when dashing
	float	decel;		// <The deceleration force applied after dashing
}RamEnemyData;

void ram_enemy_think(Entity *self) {
	if (!self || !world_get_active() || !world_get_active()->player) return;
	RamEnemyData *data = (RamEnemyData *)self->data;

	if (!data) return;

	// Get the player reference
	Entity *player_ref = world_get_active()->player;

	GFC_Vector2D direction, drag;

	if (data->timer >= data->ram_time) {
		data->timer = 0;
		gfc_vector2d_sub(direction, player_ref->position, self->position);
		gfc_vector2d_normalize(&direction);
		gfc_vector2d_scale_by(direction, direction, gfc_vector2d(data->dash_speed, data->dash_speed));
		gfc_vector2d_copy(self->velocity, direction);
	}

	if (gfc_vector2d_magnitude(self->velocity) > 10) {
		gfc_vector2d_negate(drag, self->velocity);
		gfc_vector2d_normalize(&drag);
		gfc_vector2d_scale_by(drag, drag, gfc_vector2d(data->decel, data->decel));
		gfc_vector2d_add(self->acceleration, self->acceleration, drag);
	}
}

void ram_enemy_update(Entity *self) {
	if (!self || !world_get_active() || !world_get_active()->player) return;
	RamEnemyData *data = (RamEnemyData *)self->data;
	if (!data) return;

	data->timer += 0.1;
}

void ram_enemy_touch(Entity *self, Entity *other) {
	if (!self || !other || !other->damage) return;
	other->damage(other, 5);
}

void ram_enemy_draw(Entity *self) {
	if (!self || !world_get_active() || !world_get_active()->player) return;
	RamEnemyData *data = (RamEnemyData *)self->data;
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

Entity *ram_enemy_spawn(GFC_Vector2D position, const char *config) {
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
	self->think = ram_enemy_think;
	self->update = ram_enemy_update;
	self->draw = ram_enemy_draw;
	self->touch = ram_enemy_touch;

	// Since this is a living thing, mark it as alive
	self->alive = 1;

	// Create the player data object
	RamEnemyData *data = (RamEnemyData*)malloc(sizeof(RamEnemyData));
	if (!data) {
		slog("failed to allocate memory for data");
		return NULL;
	}
	
	memset(data, 0, sizeof(RamEnemyData));

	SJson *data_json = sj_object_get_value(json, "ramEnemy");
	if (!data_json) {
		slog("%s is missing a 'ramEnemy' object", config);
		return NULL;
	}
	
	sj_object_get_float(data_json, "ramTime", &data->ram_time);
	sj_object_get_float(data_json, "dashSpeed", &data->dash_speed);
	sj_object_get_float(data_json, "decel", &data->decel);
	self->data = data;

	sj_free(data_json);
	sj_free(json);
	return self;
}

