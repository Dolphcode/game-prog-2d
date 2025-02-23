#ifndef __SPACE_H__
#define __SPACE_H__

#include "gfc_list.h"

typedef struct {
	// Bodies in the space
	GFC_List	*static_bodies;	 	// <The static bodies in the space (tiles so all rects)
	GFC_List	*physics_bodies;	// <The physics bodies in the space
}Space;

// Creating a new space

/**
 * @brief creates a new space object
 * @return NULL if failed to allocate memory, otherwise a blank space object
 */
Space *space_new();

/**
 * @brief frees a space object and all of its resources
 * @param self the space object to be freed
 */
void space_free(Space *self);

// Building out the space

/**
 * @brief adds a rect to the list of static bodies in the sapce
 * @param self the space object to be added to
 * @param shape the rect to be added
 */
void space_add_static_rect(Space *self, GFC_Rect shape);

/**
 * @brief appends the entity's physics body to the list of physics bodies in the space
 * @param self the space object to be added to
 * @param ent the entity whose body will be added
 */
void space_add_entity(Space *self, Entity *ent);

void space_remove_entity(Space *self, Entity *ent);

// Simulating the space

/**
 * @brief progresses the space's state by a timestep
 * @param self the space to be stepped
 * @param delta_time the timestep
 */
void space_step(Space *self, float delta_time);

/**
 * @brief updates the space by taking all steps
 * @param self the space to be updated
 */
void space_update(Space *self);

// Debugging the space
/**
 * @brief draw all colliders in the space
 * @param self the space to be drawn
 */
void space_draw(Space *self);

#endif
