#ifndef __FIRING_ENEMY_H__
#define __FIRING_ENEMY_H__

#include "gfc_vector.h"

#include "entity.h"

/**
 * @brief spawns an enemy that hovers a distance from the player and fires at the player
 * @param position where to spawn the enemy
 * @param config the def file for the firing enemy
 * @return a firing enemy object
 */
Entity *firing_enemy_spawn(GFC_Vector2D position, const char *config);

#endif
