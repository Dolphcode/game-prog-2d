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
	Entity 		*target;	// <For target tracking
	Entity 		*old_target;	// <For target swapping
}Camera;


/**
 * @brief Update's the main camera (Camera has built in targeting similar to Unity cinemachine
 */
void main_camera_update();

/**
 * @brief Returns the main camera's zoom value as a 2D vector
 * @return A GFC_Vector2D of the form (zoom, zoom) since zoom is a float value
 */
GFC_Vector2D main_camera_get_zoom();

/**
 * @brief Returns the main camera's zoom value as the raw zoom float value
 * @return A float copy of the main camera's zoom
 */
float main_camera_get_zoomf();

/**
 * @brief Set the camera's zoom value. Lower bounded at 0.1f
 * @param zoom the new zoom value of the camera
 */
void main_camera_set_zoom(float zoom);

/**
 * @brief returns the offset vector of the main camera
 * @return GFC_Vector2D of the camera's offset vector (-position, -position)
 */
GFC_Vector2D main_camera_get_offset();

/**
 * @brief returns the screen space draw position given a world space point
 * @param position the point to be converted in world space
 * @return the corresponding point as a GFC_Vector2D in screen space
 */
GFC_Vector2D main_camera_calc_drawpos(GFC_Vector2D position);

// DEPRECATED FUNCTIONS

/**
 * @brief updates the camera's position based on the target position
 * @param self the camera being updated
 * @deprecated camera system is now a singleton, use main_camera_update()
 */
void camera_update(Camera *self);

/**
 * @brief returns the zoom of the camera in vector form
 * @param self the camera whose zoom is being retrieved
 * @return NULL if no camera is provided, else a GFC_Vector2D of the form (zoom, zoom) (since zoom is a single float)
 * @deprecated camera system is now a singleton, use main_camera_get_zoom()
 */
GFC_Vector2D camera_get_zoom(Camera *self);

/**
 * @brief returns the offset of the camera in vector form
 * @param self the camera whose offset is being retrieved
 * @return NULL if no camera is provided, else a GFC_Vector2D with the camera's offset
 * @deprecated camera system is now a singleton, use main_camera_get_offset()
 */
GFC_Vector2D camera_get_offset(Camera *self);

/**
 * @brief get a pointer to the main camera
 * @return NULL if there is no main camera
 */
Camera *camera_get_main();
#endif
