#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_vector.h"
#include "gfc_config.h"

#include "weapon.h"
/*
 *typedef struct {
	const char	*name;	// <Track the weapon's name
	WeaponClass	type;	// <This weapon's weapon class
	
	// Weapon visuals
	Sprite		*sprite;// <This weapon's sprite for rendering the weapon

	// Generic weapon info
	float		rate;		// <Time between each fire call
	
	// Ranged weapon info
	float		count;		// <Number of projectiles fired in one shot
	float		inaccuracy;	// <Randomness in the weapon's fire
	float		spread;		// <If count > 1, how spread out should projectiles be
	float		proj_speed;	// <Speed at which projectiles are launched
	const char	projectile[256];// <Which projectile spawn function should be invoked
	


	// Weapon state
	float		fire_time;	// <Time since last fire call
}Weapon;*/


void weapon_load_from_file(Weapon *self, const char *path) {
	if (!path) return;
	SJson *json = sj_load(path);
	weapon_load(self, json);
	sj_free(json);
}

void weapon_load(Weapon *self, SJson *data) {
	if (!self || !data) return;
	const char *sprite = NULL, *projectile = NULL;
	GFC_Vector2D frame_size, sprite_offset;
	Uint32 fpl;

	self->type = WC_RANGE;

	sj_object_get_int(data, "count", &self->count);
	sj_object_get_float(data, "inaccuracy", &self->inaccuracy);
	sj_object_get_float(data, "spread", &self->spread);
	sj_object_get_float(data, "rate", &self->rate);
	sj_object_get_vector2d(data, "spawnOffset", &self->spawn_offset);

	projectile = sj_object_get_string(data, "projectile");
	strcpy(self->projectile, projectile);
	
	sprite = sj_object_get_string(data, "sprite");
	if (sprite) {
		sj_object_get_vector2d(data, "spriteSize", &frame_size);
		sj_object_get_uint32(data, "spriteFPL", &fpl);
		self->sprite = gf2d_sprite_load_all(
			sprite,
			(Uint32)frame_size.x,
			(Uint32)frame_size.y,
			fpl,
			0);

		sj_object_get_vector2d(data, "spriteOffset", &sprite_offset);
		self->sprite_offset = sprite_offset;
	}
}
