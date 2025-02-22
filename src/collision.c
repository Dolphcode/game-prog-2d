#include "simple_logger.h"

#include "collision.h"

/*
typedef struct {
	GFC_Vector2D	poc;	// <The point of collision
	GFC_Vector2D	normal;	// <The normal vector for the collision
}Collision;
*/

/**
 * @brief free a collision list
 * @param the list object to be freed
 */
void collision_list_free(GFC_List *list) {
	// Validate the pointer
	if (!list) return;
	int i, count;
	Collision* curr;

	// Free each collision individually
	count = gfc_list_count(list);
	for (i = 0; i < count; ++i) {
		curr = (Collision*)gfc_list_get_nth(list, i);
		if (curr) {
			collision_free(curr);
		}
	}

	// Delete the list
	gfc_list_delete(list);
}

/**
 * @brief allocate memory for a new collision object
 * @return NULL if failed to allocate memory, otherwise a collision object
 */
Collision *collision_new() {
	// Create the new collision object
	Collision *collision = gfc_allocate_array(sizeof(GFC_Vector2D), 1);

	// Double checking collision
	if (!collision) {
		slog("failed to allocate memory for collision");
		return NULL;
	}

	// Set the memory to 0 just in case and return
	memset(collision, 0, sizeof(Collision));
	return collision;
}

/**
 * @brief free the collision object
 * @param collision the collision object to be freed
 */
void collision_free(Collision *collision) {
	if (!collision) return;
	free(collision);
}
