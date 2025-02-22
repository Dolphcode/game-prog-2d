#ifndef __SPACE_H__
#define __SPACE_H__

#include "gfc_text.h"
#include "gfc_list.h"
#include "gfc_shape.h"

#include "entity.h"

typedef struct {

	// Debug stuff
	GFC_TextLine	name;		//<The name of the space for debugging purposes
	
	// Physics bodies in the space
	GFC_List	*static_shapes;	//<List of all static shapes in the physics space

}Space;

/**
 * @brief allocate memory for a physics space object
 * @return NULL if failure, otherwise a new space object
 * @note this object must be freed manually as there is no space manager/list
 */
Space* space_new();

/**
 * @brief free a space object and all of the resources it consumes
 * @param self the space object to be freed
 */
void space_free(Space *self);

/**
 * @brief add a static shape to the list of static shapes in the space
 * @param self the space that a shape will be added to
 * @param shape the shape data to be appended into the GFC_List
 */
void space_add_static_shape(Space *self, GFC_Shape shape);

/**
 * @brief for debugging purposes, draws all static shapes in the space
 */
void space_draw(Space *self);

// Collision/overlap checking

/**
 * @brief check if an entity is overlapping with any static shape in the space
 * @param entity the entity whose bounds are being checked with static shapes in the world
 * @return a list of shape overlaps as Vector2Ds
 * @note this list is not freed on its own, and must be freed by the function caller
 */
GFC_List *space_overlap_entity_static_shape(Space *self, Entity *entity);

#endif
