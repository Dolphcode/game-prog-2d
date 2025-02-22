#include "simple_logger.h"

#include "gfc_input.h"

#include "gf2d_graphics.h"

#include "camera.h"

static Camera main_camera = {0}; // May be temporary, might assign main camera to world instead

/**
 * @brief Update's the main camera (Camera has built in targeting similar to Unity cinemachine
 */
void main_camera_update() {
	camera_update(&main_camera);
}

/**
 * @brief Returns the main camera's zoom value as a 2D vector
 * @return A GFC_Vector2D of the form (zoom, zoom) since zoom is a float value
 */
GFC_Vector2D main_camera_get_zoom() {
	return gfc_vector2d(main_camera.zoom, main_camera.zoom);
}

/**
 * @brief Returns the main camera's zoom value as the raw zoom float value
 * @return A float copy of the main camera's zoom
 */
float main_camera_get_zoomf() {
	return main_camera.zoom;
}

/**
 * @brief Set the camera's zoom value. Lower bounded at 0.1f
 * @param zoom the new zoom value of the camera
 */
void main_camera_set_zoom(float zoom) {
	main_camera.zoom = zoom;
}

GFC_Vector2D main_camera_calc_drawpos(GFC_Vector2D position) {
	GFC_Vector2D draw_pos = {0};
	gfc_vector2d_add(draw_pos, position, main_camera_get_offset());

	gfc_vector2d_scale_by(draw_pos, draw_pos, main_camera_get_zoom());

	GFC_Vector2D screen_res = gf2d_graphics_get_resolution();
	gfc_vector2d_scale_by(screen_res, screen_res, gfc_vector2d(0.5, 0.5));
	gfc_vector2d_add(draw_pos, draw_pos, screen_res);

	return draw_pos;
}


/**
 * @brief returns the offset vector of the main camera
 * @return GFC_Vector2D of the camera's offset vector (-position, -position)
 */
GFC_Vector2D main_camera_get_offset() {
	return gfc_vector2d(-main_camera.position.x, -main_camera.position.y);
}

Camera* camera_get_main() {
	return &main_camera;
}

void camera_update(Camera *self) {
	// Check if we have a valid pointer
	if (!self) return;

	// Check if we have a target
	if (!self->target) return;
	
	// check input and configure zoom
	if (gfc_input_command_down("zoom_in")) {
		self->zoom += 0.02;
	}
	if (gfc_input_command_down("zoom_out")) {
		self->zoom -= 0.02;
	}

	// Lower bound zoom
	if (self->zoom < 0.1) self->zoom = 0.1;

	// Get screen resolution
	GFC_Vector2D screen_res = gf2d_graphics_get_resolution();

	// Update position
	self->position = self->target->position;

	// Update the rect
	self->bounds.x = self->position.x - screen_res.x / 2.0;
	self->bounds.y = self->position.y - screen_res.y / 2.0;
	self->bounds.w = screen_res.x;
	self->bounds.h = screen_res.y;
}

GFC_Vector2D camera_get_zoom(Camera *self) {
	return gfc_vector2d(self->zoom, self->zoom);
}

GFC_Vector2D camera_get_offset(Camera *self) {
	return gfc_vector2d(-self->position.x, -self->position.y);
}


