#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_vector.h"
#include "gfc_shape.h"

#include "entity.h"

typedef struct Camera_S {
	// Spatial information
	GFC_Vector2D	position;	// <The camera's position
	GFC_Rect	bounds;		// <The bounds for drawing things within the camera
	float		zoom;		// <The camera's zoom factor (scales game objects, not UI)

	// Entity targeting references
	Entity 		*target;
	Entity 		*old_target;
}Camera;

/**
 * @brief allocates memory for a new camera object
 * @param sets this camera instance to the main camera instance if set to true
 * @return NULL if failed to allocate memory
 */
Camera *camera_new(Uint8 set_main);

/**
 * @brief frees the camera from memory
 * @param self the camera instance being freed
 */
void camera_free(Camera *self);

/**
 * @brief updates the camera's position based on the target position
 * @param self the camera being updated
 */
void camera_update(Camera *self);

/**
 * @brief get a pointer to the main camera
 * @return NULL if there is no main camera
 */
Camera *camera_get_main();
#endif
