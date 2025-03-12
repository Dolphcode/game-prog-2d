#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "simple_json.h"

#include "gfc_vector.h"

#include "gf2d_sprite.h"

typedef enum {
	WC_RANGE = 0,
	WC_MELEE = 1,
	WC_DRONE = 2
}WeaponClass;

typedef struct {
	const char	*name;	// <Track the weapon's name
	WeaponClass	type;	// <This weapon's weapon class
	
	// Weapon visuals
	Sprite		*sprite;// <This weapon's sprite for rendering the weapon
	GFC_Vector2D	sprite_offset;

	// Generic weapon info
	float		rate;		// <Time between each fire call
	
	// Ranged weapon info
	int		count;		// <Number of projectiles fired in one shot
	float		inaccuracy;	// <Randomness in the weapon's fire
	float		spread;		// <If count > 1, how spread out should projectiles be
	float		proj_speed;	// <Speed at which projectiles are launched
	const char	projectile[256];// <Which projectile spawn function should be invoked
	GFC_Vector2D	spawn_offset;	// <Offset with which projectiles are spawned
	
	// Weapon state
	float		fire_time;	// <Time since last fire call
	float		use_time;	// <Time until weapon is no longer being used (for melee primarily);
	GFC_Vector2D	direction;	// <The direction of the weapon
	float		rotation;	// <The rotation of the weapon
}Weapon;

void weapon_load_from_file(Weapon *self, const char *path);

/**
 * @brief loads the weapon data
 * @param self the weapon struct whose data members should be loaded
 * @param config the json object used to load this data
 */
void weapon_load(Weapon *self, SJson *config);

#endif
