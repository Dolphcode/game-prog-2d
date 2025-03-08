#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

/**
 * @brief creats a new player entity
 * @param position where to spawn it
 * @return NULL on error
 */
Entity *player_new_entity(GFC_Vector2D position);

/**
 * @brief spawns the player entity and the player hook
 * @param position the position where the player should be spawned
 * @param config used to determine the player's equipped weapon
 * @return NULL on error, otherwise a new player entity
 */
Entity *player_spawn(GFC_Vector2D position, const char* config);

/**
 * @brief draws the player's hud
 */
void player_hud_draw();

void player_hud_init();

void player_hud_close();

#endif
