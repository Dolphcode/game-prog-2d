#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

typedef struct {
	
	

}ProjectileData;

void projectile_pool_init();

/**
 *
 */
void projectile_pool_close();

/**
 * @brief clears the pool of projectiles
 */
void projectile_pool_clear();


#endif
