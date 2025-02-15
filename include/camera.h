#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_vector.h"
#include "gfc_shape.h"

#include "entity.h"

typedef struct Camera_S {
	// Spatial information
	GFC_Vector2D	position;	// <The camera's position
	GFC_Rect	bounds;		// <The camera's bounds in space
	float		zoom;		// <The camera's zoom factor (scales game objects, not UI)

	// Entity targeting references
	Entity 		*target;
	Entity 		*old_target;
}Camera;

/**
 * @brief updates the camera's position based on the target position
 * @param self the camera being updated
 */
void camera_update(Camera *self);

/**
 * @brief returns the zoom of the camera in vector form
 * @param self the camera whose zoom is being retrieved
 * @return NULL if no camera is provided, else a GFC_Vector2D of the form (zoom, zoom) (since zoom is a single float)
 */
GFC_Vector2D camera_get_zoom(Camera *self);

/**
 * @brief returns the offset of the camera in vector form
 * @param self the camera whose offset is being retrieved
 * @return NULL if no camera is provided, else a GFC_Vector2D with the camera's offset
 */
GFC_Vector2D camera_get_offset(Camera *self);

/**
 * @brief get a pointer to the main camera
 * @return NULL if there is no main camera
 */
Camera *camera_get_main();
#endif
