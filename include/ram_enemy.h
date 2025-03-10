#ifndef __RAM_ENEMY_H__
#define __RAM_ENEMY_H__

#include "gfc_vector.h"

#include "entity.h"

/**
 * @brief spawns a ram enemy
 * @param position where to spawn the ram enemy
 * @param config the def file
 * @return NULL if fail, otherwise a ram enemy
 */
Entity *ram_enemy_spawn(GFC_Vector2D position, const char *config);

#endif
