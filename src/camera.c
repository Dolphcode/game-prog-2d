#include "simple_logger.h"

#include "gfc_input.h"

#include "gf2d_graphics.h"

#include "camera.h"

static Camera* main_camera; // May be temporary, might assign main camera to world instead

/**
 * @brief frees the main camera if it exists
 */
void camera_free_main();

Camera *camera_new(Uint8 set_main) {
	// Allocate memory
	Camera* ptr = (Camera*) malloc(sizeof(Camera));

	// Check if we should set main
	if (set_main) main_camera = ptr;

	// Initialize zoom
	ptr->zoom = 1.0;

	// Temporary
	atexit(camera_free_main);

	return ptr;
}

void camera_free(Camera *self) {
	// Check the pointer
	if (!self) return;

	free(self);
}

void camera_free_main() {
	if (!main_camera) return;
	camera_free(main_camera);
	slog("main camera freed");
}

Camera* camera_get_main() {
	if (!main_camera) return NULL;
	return main_camera;
}

void camera_update(Camera *self) {
	// Check if we have a valid pointer
	if (!self) return;

	// Check if we have a target
	if (!self->target) return;
	
	// check input and configure zoom
	if (gfc_input_command_down("zoom_in")) {
		self->zoom += 0.1;
		slog("in");
	}
	if (gfc_input_command_down("zoom_out")) {
		self->zoom -= 0.1;
		slog("out");
	}

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


