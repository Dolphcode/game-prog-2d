#include <math.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_list.h"

#include "gf2d_sprite.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"

#include "camera.h"
#include "entity.h"
#include "projectile.h"

/*
typedef struct {
	Uint8	is_active;	// <Whether the projectile is active or not
	float	lifetime;	// <How long this projectile should stay alive for
	float	timer;		// <A timer used to keep track of how long this projectile has been alive for
}ProjectileData;
*/

typedef struct {
	const char *name;
	const char *config;
}ProjectileConfig;

static ProjectileConfig projectile_list[] = {
	{
		"turret_shot",
		"def/projectile/turret_shot.def"	
	},
	{
		"bullet",
		"def/projectile/bullet.def"
	},
	{
		"fire",
		"def/projectile/fire.def"
	},
	{
		"spikeball",
		"def/projectile/spikeball.def"
	},
	{
		"rocket",
		"def/projectile/rocket.def"
	},
	{
		"moabrocket",
		"def/projectile/moabrocket.def"
	},
	{
		"sword_hit",
		"def/projectile/sword_hit.def"
	},
	{0}
};

static GFC_List *projectile_pool = NULL;

void projectile_pool_init() {
	// Create the projectile pool
	projectile_pool = gfc_list_new();
	if (!projectile_pool) {
		slog("failed to create projectile pool");
	}

	atexit(projectile_pool_close);
}

void projectile_pool_close() {
	projectile_pool_clear();
	gfc_list_delete(projectile_pool);
}

void projectile_pool_clear() {
	if (!projectile_pool) return;
	int i, count = gfc_list_count(projectile_pool);
	Entity *curr;

	for (i = count - 1; i >= 0; --i) {
		curr = gfc_list_get_nth(projectile_pool, i);
		// Free this projectile if it is still in use
		/*
		if (curr && curr->_inuse) {
			entity_free(curr);
			slog("alright feeling like freeing");
		}
		slog("trying to delete this thing");*/
		// Delete this list element
		gfc_list_delete_nth(projectile_pool, i);
	}
}

void projectile_touch(Entity *self, Entity *other) {
	if (!self || !other || !other->damage || !other->alive || self->team == other->team) return;
	ProjectileData *data = (ProjectileData *)self->data;
	if (!data || !data->is_active) return;

	other->damage(other, 2.0); // Temporary, add a contact damage component
	
	if (!data->pierce) {
		data->is_active = 0;
		self->body->disabled = 1;
		self->do_draw = 0;
	}
}

void projectile_update(Entity *self) {
	if (!self) return;
	ProjectileData *data = (ProjectileData *)self->data;
	if (!data) return;

	if (data->timer < data->lifetime) {
		data->timer += 0.1;
	} else {
		data->is_active = 0;
		self->body->disabled = 1;
		self->do_draw = 0;
	}
}

void projectile_draw(Entity *self) {
	// Verify pointers
	if (!self || !self->sprite) return;
	ProjectileData *data = (ProjectileData *)self->data;
	if (!data || !data->is_active) return;

	// Calculate draw position and scale
	GFC_Vector2D scale = main_camera_get_zoom();

	GFC_Vector2D draw_pos = main_camera_calc_drawpos(self->position);

	GFC_Vector2D center = self->sprite_offset;

	data->rot = gfc_vector2d_angle(self->velocity) * 180 / M_PI - 90.0;
	
	if (strcmp(self->name, "sword_hit") == 0) return;
	// Draw the sprite
	gf2d_sprite_draw(
		self->sprite,
		draw_pos,
		&scale,
		&center,
		&data->rot,
		NULL,
		NULL,
		(Uint32)self->frame);

	// Draw the point
	if (DRAW_CENTER) gf2d_draw_circle(draw_pos, 4, GFC_COLOR_LIGHTGREEN);
}

void projectile_gravity_think(Entity *self) {
	if (!self) return;
	self->acceleration.y += 500;
}

void projectile_speed_up_think(Entity *self) {
	if (!self) return;
	GFC_Vector2D force;
	gfc_vector2d_copy(force, self->velocity);
	gfc_vector2d_normalize(&force);
	gfc_vector2d_scale_by(force, force, gfc_vector2d(100, 100));
	gfc_vector2d_add(self->acceleration, self->acceleration, force);
}

Entity *projectile_new(const char* name) {
	ProjectileConfig *conf;
	Entity *proj;
	if (!name) {
		slog("no spawn name provided");
		return NULL;
	}

	for (conf = projectile_list; conf->name != 0; ++conf) {
		if (strcmp(name, conf->name) == 0) {
			// Get the projectile
			proj = entity_new();
			if (!proj) {
				slog("failed to retrieve entity slot");
				return NULL;
			}

			// Load the entity data
			SJson *json = sj_load(conf->config);
			if (!json) {
				slog("failed to open def file");
				return NULL;
			}
			entity_configure(proj, json);

			// Get the projectile def file
			SJson *proj_json = sj_object_get_value(json, "projectile");
			if (!proj_json) {
				slog("couldn't find 'projectile' object in def file");
				return NULL;
			}
			
			// Load the projectile data
			ProjectileData *proj_data = (ProjectileData*)malloc(sizeof(ProjectileData));
			if (!proj_data) {
				slog("failed to allocate memory for projectile data");
				return NULL;
			}
			memset(proj_data, 0, sizeof(ProjectileData));
			sj_object_get_float(proj_json, "lifetime", &proj_data->lifetime);
			sj_object_get_uint8(proj_json, "pierce", &proj_data->pierce);
			proj_data->is_active = 1;

			proj->data = proj_data;

			// Assign functions
			proj->update = projectile_update;
			proj->touch = projectile_touch;
			proj->draw = projectile_draw;

			Uint8 type = 0;
			sj_object_get_uint8(proj_json, "type", &type);
			if (type == 1) {
				proj->think = projectile_gravity_think;
			} else if (type == 2) {
				proj->think = projectile_speed_up_think;
			}
			
			// Add it to the projectile pool
			gfc_list_append(projectile_pool, proj);
			
			// Return the projectile object
			return proj;
		}
	}

	slog("failed to spawn projectile");
	return NULL;
}

Entity *projectile_spawn(const char *name) {
	Entity *proj;
	ProjectileData *data;
	int i, count = gfc_list_count(projectile_pool);
	// Check the projectile pool for an inactive matching projectile
	for (i = 0; i < count; ++i) {
		proj = gfc_list_get_nth(projectile_pool, i);
		if (!proj) continue;

		if (strcmp(proj->name, name) == 0) {
			data = (ProjectileData *)proj->data;
			if (!data) continue;

			if (!data->is_active) { // If this is a matching inactive projectile, reactivate it
				data->is_active = 1;
				data->timer = 0;
				proj->body->disabled = 0;
				proj->do_draw = 1;
				return proj;
			}
		}
	}

	// Failed to find projectile in the pool, create it
	proj = projectile_new(name);
	if (!proj) {
		slog("failed to create projectile %s", name);
		return NULL;
	}
	return proj;
}

Entity *projectile_fire(const char *name, GFC_Vector2D position, GFC_Vector2D velocity) {
	Entity *proj = projectile_spawn(name);

	if (!proj) {
		slog("failed to spawn projectils %s", name);
		return NULL;
	}

	gfc_vector2d_copy(proj->position, position);
	gfc_vector2d_copy(proj->velocity, velocity);
	return proj;
}

void projectile_fire_ex(const char *name, GFC_Vector2D position, GFC_Vector2D direction, float speed, float spread, float inaccuracy, int count) {
	if (count < 1) {
		slog("cannot fire less than 1 projectile");
		return;
	}

	if (inaccuracy > 0) {
		inaccuracy = inaccuracy / 180 * M_PI;
		float amount = -inaccuracy + (2 * inaccuracy) * ((float)rand() / (float)RAND_MAX);
		direction = gfc_vector2d_rotate(direction, amount);
	}

	GFC_Vector2D velocity;
	float start_rot, rot_interval;
	gfc_vector2d_scale_by(velocity, direction, gfc_vector2d(speed, speed));

	if (count == 1) {
		projectile_fire(name, position, velocity);	
	} else {
		start_rot = -spread * 0.5 / 180 * M_PI;
		rot_interval = (spread / ((float)(count - 1))) / 180 * M_PI;
		velocity = gfc_vector2d_rotate(velocity, start_rot);
		for (int i = 0; i < count; ++i) {
			projectile_fire(name, position, velocity);
			velocity = gfc_vector2d_rotate(velocity, rot_interval);
		}

	}
}	
