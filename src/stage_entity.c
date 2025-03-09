#include <math.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "gf2d_sprite.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"

#include "camera.h"
#include "stage_entity.h"
#include "projectile.h"

// TEMPORARY PLATFORM STAGE HAZARD

typedef struct {
	float	lifetime;	// <How long this object can be grappled to before it collapses
	float	cooldown;	// <How long this object takes to return to its grappleable state
	float	timer;		// <A timer object for this temporary platform	
	Uint8	grappled;	// <Whether something is grappled to this object or not
}TempPlatformData;

void temp_platform_touch(Entity *self, Entity *other) {
	if (!self) return;
	TempPlatformData *data = (TempPlatformData*)self->data;
	if (!data) return;

	if (strcmp(other->name, "hook") == 0 && self->can_grapple) {
		data->grappled = 1;
	}
}

void temp_platform_think(Entity *self) {
	if (!self) return;
	TempPlatformData *data = (TempPlatformData*)self->data;
	if (!data) return;
	
	if (!self->can_grapple) {
		self->frame = 2;
	} else if (data->grappled) {
		self->frame = 1;
	} else {
		self->frame = 0;
	}

	// Force a recheck for if we are still grappled to
	data->grappled = 0;

}

void temp_platform_update(Entity *self) {
	if (!self) return;
	TempPlatformData *data = (TempPlatformData*)self->data;
	if (!data) return;


	if (self->can_grapple) {
		if (data->timer > 0 && data->grappled) {
			data->timer -= 0.1;
		} else if (data->timer <= 0) {
			data->timer = data->cooldown;
			self->can_grapple = 0;
		}
	} else {
		if (data->timer > 0) {
			data->timer -= 0.1;
		} else {
			self->can_grapple = 1;
			data->timer = data->lifetime;
		}
	}
}

Entity *spawn_temp_platform(GFC_Vector2D position, const char *config) {
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to allocate memory for temporary platform");
		return NULL;
	}

	gfc_vector2d_copy(self->position, position);

	SJson *json = sj_load("./def/temp_hook.def");
	if (!json) {
		slog("failed to open def file");
		return NULL;
	}
	entity_configure(self, json);

	TempPlatformData *data = (TempPlatformData*)malloc(sizeof(TempPlatformData));
	if (!data) {
		slog("failed to allocate memory for data");
		return NULL;
	}
	memset(data, 0, sizeof(TempPlatformData));

	SJson *data_json = sj_object_get_value(json, "tempPlatformData");
	if (!data_json) {
		slog("couldn't find tempPlatformData object in def file");
		return NULL;
	}
	sj_object_get_float(data_json, "lifetime", &data->lifetime);
	sj_object_get_float(data_json, "cooldown", &data->cooldown);
	data->timer = data->lifetime;

	self->data = data;

	self->touch = temp_platform_touch;
	self->think = temp_platform_think;
	self->update = temp_platform_update;
	
	self->can_grapple = 1;
	return self;
}

// BUZZSAW STAGE HAZARD

void buzzsaw_touch(Entity *self, Entity *other) {
	if (!self || !other || !other->damage) return;
	other->damage(other, 4.0);
}

void buzzsaw_update(Entity *self) {
	self->frame += 0.1;
	if (self->frame > 3) {
		self->frame = 0;
	}
}

Entity* spawn_buzzsaw(GFC_Vector2D position, const char *config) {
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to allocate memory for temporary platform");
		return NULL;
	}

	gfc_vector2d_copy(self->position, position);

	SJson *json = sj_load("./def/buzzsaw.def");
	if (!json) {
		slog("failed to open def file");
		return NULL;
	}
	entity_configure(self, json);

	self->touch = buzzsaw_touch;
	self->update = buzzsaw_update;

	return self;
}

// MOVING GRAPPLE STAGE HAZARD


// TURRET STAGE HAZARD
typedef struct {
	float	rate;		// <This turret's rate of fire
	float	timer;		// <The time to the next fire
	float	rot;		// <The rotation of the turret
	Uint8	grappled;	// <Whether this object is grappled to at the moment or not
}TurretData;

void turret_proj_touch(Entity *self, Entity *other) {
	if (!self || !other || !other->damage) return;
	other->damage(other, 3);
}

Entity *spawn_turret_proj(GFC_Vector2D position) {
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to allocate memory for temporary platform");
		return NULL;
	}

	gfc_vector2d_copy(self->position, position);

	SJson *json = sj_load("./def/turret_proj.def");
	if (!json) {
		slog("failed to open def file");
		return NULL;
	}
	entity_configure(self, json);

	self->touch = turret_proj_touch;

	return self;
}

void turret_update(Entity *self) {
	if (!self) return;
	TurretData* data = (TurretData *)self->data;
	if (!data) return;

	if (data->timer > 0) {
		data->timer -= 0.1;
	} else {
		data->timer = data->rate;
		if (!data->grappled) {
			GFC_Vector2D dir = {0, 1};
			dir = gfc_vector2d_rotate(dir, -data->rot);
			GFC_Vector2D velocity;
			for (int i = 0; i < 4; ++i) {
				//Entity *proj = spawn_turret_proj(self->position);
				gfc_vector2d_scale_by(velocity, dir, gfc_vector2d(200, 200));
				projectile_fire("turret_shot", self->position, velocity);
				dir = gfc_vector2d_rotate(dir, 0.5 * M_PI);
			}
		}
	}

	data->rot += 0.1;

	// Force recheck
	data->grappled = 0;
}

void turret_touch(Entity *self, Entity *other) {
	if (!self) return;
	TurretData* data = (TurretData *)self->data;
	if (!data) return;
	if (strcmp(other->name, "hook") == 0) {
		data->grappled = 1;
	}
}

void turret_draw(Entity *self) {
	if (!self) return;
	TurretData* data = (TurretData *)self->data;
	if (!data) return;
	// Calculate draw position and scale
	GFC_Vector2D scale = main_camera_get_zoom();
	GFC_Vector2D draw_pos = main_camera_calc_drawpos(self->position);

	GFC_Vector2D center = self->sprite_offset;

	// Draw the guns
	gf2d_sprite_draw(
		self->sprite,
		draw_pos,
		&scale,
		&center,
		&data->rot,
		NULL,
		NULL,
		1);

	// Draw the sprite
	gf2d_sprite_draw(
		self->sprite,
		draw_pos,
		&scale,
		&center,
		NULL,
		NULL,
		NULL,
		0);

	// Draw the point
	if (DRAW_CENTER) gf2d_draw_circle(draw_pos, 4, GFC_COLOR_LIGHTGREEN);
}

Entity *spawn_turret(GFC_Vector2D position, const char *config) {
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to allocate memory for temporary platform");
		return NULL;
	}

	gfc_vector2d_copy(self->position, position);

	SJson *json = sj_load("./def/turret.def");
	if (!json) {
		slog("failed to open def file");
		return NULL;
	}
	entity_configure(self, json);

	TurretData *data = (TurretData*)malloc(sizeof(TurretData));
	if (!data) {
		slog("failed to allocate memory for data");
		return NULL;
	}
	memset(data, 0, sizeof(TurretData));

	SJson *data_json = sj_object_get_value(json, "turretData");
	if (!data_json) {
		slog("couldn't find turretData object in def file");
		return NULL;
	}
	sj_object_get_float(data_json, "rate", &data->rate);
	data->timer = data->rate;

	self->data = data;
	self->can_grapple = 1;

	self->draw = turret_draw;
	self->update = turret_update;
	self->touch = turret_touch;
	
	return self;
}
