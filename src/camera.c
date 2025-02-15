#include "simple_logger.h"

#include "gfc_input.h"

#include "gf2d_graphics.h"

#include "camera.h"

static Camera main_camera = {0}; // May be temporary, might assign main camera to world instead

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
		slog("in");
	}
	if (gfc_input_command_down("zoom_out")) {
		self->zoom -= 0.02;
		slog("out");
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


