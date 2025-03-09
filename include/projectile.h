#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

/* This is to provide support for projectiles and projectile pooling as 
 * projectiles will be rapidly created and destroyed at runtime
 *
 * This header will also include common projectile functionality
 */

typedef struct {
	Uint8	is_active;	// <Whether the projectile is active or not
	float	lifetime;	// <How long this projectile should stay alive for
	float	timer;		// <A timer used to keep track of how long this projectile has been alive for
	float	rot;		// <The rotation value of the projectile
}ProjectileData;

/**
 * @brief initializes the projectile pool
 */
void projectile_pool_init();

/**
 * @brief closes the projectile pool
 */
void projectile_pool_close();

/**
 * @brief clears the pool of projectiles
 */
void projectile_pool_clear();

/**
 * @brief fires a projectile at a given position with a given velocity
 * @param name which projectile to spawn
 * @param position where the projectile should be spawned
 * @param velocity the initial velocity vector of the projectile
 */
Entity *projectile_fire(const char *name, GFC_Vector2D position, GFC_Vector2D velocity);

#endif
