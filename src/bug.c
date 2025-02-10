#include "simple_logger.h"

#include "gf2d_sprite.h"
#include "gf2d_graphics.h"

#include "bug.h"
#include "camera.h"

void bug_draw(Entity *self) {
	// Verify pointers
	if (!self) return;
	if (!self->sprite) return;

	// Calculate drawpos, center, and scale
	GFC_Vector2D drawpos = {0};
	gfc_vector2d_sub(drawpos, self->position, camera_get_main()->position);
	gfc_vector2d_sub(drawpos, drawpos, gfc_vector2d(self->sprite->frame_w / 2.0, self->sprite->frame_h / 2.0));
	gfc_vector2d_scale_by(drawpos, drawpos, gfc_vector2d(camera_get_main()->zoom, camera_get_main()->zoom));
	GFC_Vector2D screen_res = gf2d_graphics_get_resolution();
	screen_res.x /= 2.0;
	screen_res.y /= 2.0;
	gfc_vector2d_add(drawpos, drawpos, screen_res);
	
	GFC_Vector2D scale = gfc_vector2d(camera_get_main()->zoom, camera_get_main()->zoom);
	GFC_Vector2D center = {0};
	gfc_vector2d_sub(center, drawpos, screen_res);
	gfc_vector2d_negate(center, center);
	//gfc_vector2d_scale_by(center, gf2d_graphics_get_resolution(), gfc_vector2d(0.5, 0.5));

	// Draw the player sprite
	gf2d_sprite_draw(
		self->sprite,
		drawpos,
		&scale,
		NULL,
		NULL,
		NULL,
		NULL,
		(Uint32)self->frame);

	// Update the frame
	self->frame += 0.1;
	if (self->frame >= 16.0) {
		self->frame = 0.0;
	}
}

void bug_think(Entity *self) {

}

void bug_update(Entity *self) {
	if (!self) return;

	self->lifetime += 0.1;
	if (self->lifetime > 15) {
		entity_free(self);
		return;
	}

	gfc_vector2d_add(self->position, self->position, self->velocity);
}

Entity *bug_new_entity(GFC_Vector2D position, const char *filename) {
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to spawn a new bug entity");
		return NULL;
	}

	// Copy position data
	gfc_vector2d_copy(self->position, position);

	// Initialize and assign sprite
	entity_configure_from_file(self, filename);

	// Assign functions
	self->draw = bug_draw;
	self->think = bug_think;
	self->update = bug_update;

	return self;
}
