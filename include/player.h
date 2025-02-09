#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

/**
 * @brief spawn a new player entity
 * @param position where to spawn it
 * @return NULL on error
 */
Entity *player_new_entity(GFC_Vector2D position);

#endif
