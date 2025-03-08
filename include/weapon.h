#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "gfc_vector.h"

typedef enum {
	WC_RANGE = 0,
	WC_MELEE = 1,
	WC_DRONE = 2
}WeaponClass;

typedef struct {
	const char	*name;	// <Track the weapon's name
	WeaponClass	type;	// <This weapon's weapon class

	// Generic weapon info
	float		rate;		// <Time between each fire call
	
	// Ranged weapon info
	float		count;		// <Number of projectiles fired in one shot
	float		precision;	// <Randomness in the weapon's fire
	float		spread;		// <If count > 1, how spread out should projectiles be
	const char	*projectile;	// <Which projectile spawn function should be invoked
	
	// Weapon state
	float		fire_time;	// <Time since last fire call
}Weapon;




#endif
