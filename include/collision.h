#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "gfc_list.h"
#include "gfc_vector.h"

typedef struct {
	GFC_Vector2D	poc;	// <The point of collision
	GFC_Vector2D	normal;	// <The normal vector for the collision
}Collision;

/**
 * @brief free a collision list
 * @param the list object to be freed
 */
void collision_list_free(GFC_List *list);

/**
 * @brief allocate memory for a new collision object
 * @return NULL if failed to allocate memory, otherwise a collision object
 */
Collision *collision_new();

/**
 * @brief free the collision object
 * @param collision the collision object to be freed
 */
void collision_free(Collision *collision);
#endif
