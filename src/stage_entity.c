#include "simple_logger.h"
#include "simple_json.h"

#include "stage_entity.h"

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

/**
 * @brief spawns a buzzsaw in the world (stage hazard)
 * @param position the position of the top left corner of the buzzsaw
 * @config the configuration file of the buzzsaw
 */
Entity* spawn_buzzsaw(GFC_Vector2D position, const char *config);


