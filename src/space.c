#include "simple_logger.h"

#include "gfc_color.h"

#include "gf2d_draw.h"
#include "gf2d_graphics.h"

#include "camera.h"
#include "space.h"
#include "collision.h"

/*
typedef struct {

	// Debug stuff
	GFC_TextLine	name;		//<The name of the space for debugging purposes
	
	// Physics bodies in the space
	GFC_List	static_shapes;	//<List of all static shapes in the physics space

}Space;*/

Space* space_new() {
	// Allocate memory
	Space *space;
	space = gfc_allocate_array(sizeof(Space), 1);

	// Verify the space pointer
	if (!space) {
		slog("failed to create space object");
		return NULL;
	}

	// Create the static shape list
	space->static_shapes = gfc_list_new();

	// Create the body shape list
	space->bodies = gfc_list_new();

	return space;
}

void space_free(Space *self) {
	// Free the list of static shapes
	Uint32 i, list_count;
	list_count = gfc_list_get_count(self->static_shapes);
	for (i = 0; i < list_count; ++i) {
		free(gfc_list_get_nth(self->static_shapes, i));
	}

	// Free the list of bodies
	list_count = gfc_list_get_count(self->bodies);
	for (i = 0; i < list_count; ++i) {
		body_free(gfc_list_get_nth(self->bodies, i));
	}

	gfc_list_delete(self->static_shapes);
	gfc_list_delete(self->bodies);
	free(self);	
}

void space_add_static_shape(Space *self, GFC_Shape shape) {
	GFC_Shape *shape_mem;
	shape_mem = gfc_allocate_array(sizeof(GFC_Shape), 1);

	gfc_shape_copy(shape_mem, shape);

	gfc_list_append(self->static_shapes, shape_mem);
}

/**
 * @brief for debugging purposes, draws all static shapes in the space
 */
void space_draw(Space *self) {
	Uint32 i, count;
	GFC_Shape *curr;
	count = gfc_list_get_count(self->static_shapes);

	GFC_Vector2D screen_res = gf2d_graphics_get_resolution();
	gfc_vector2d_scale_by(screen_res, screen_res, gfc_vector2d(0.5, 0.5));

	for (i = 0; i < count; ++i) {
		curr = gfc_list_get_nth(self->static_shapes, i);

		GFC_Vector2D scale = main_camera_get_zoom();
		GFC_Vector2D draw_pos = {0};

		if (curr->type == ST_RECT) {
			gfc_vector2d_add(draw_pos, gfc_vector2d(curr->s.r.x, curr->s.r.y), main_camera_get_offset());
			gfc_vector2d_scale_by(draw_pos, draw_pos, scale);
			gfc_vector2d_add(draw_pos, draw_pos, screen_res);

			GFC_Rect rect = gfc_rect(draw_pos.x, draw_pos.y, scale.x * curr->s.r.w, scale.y * curr->s.r.h);
			gf2d_draw_rect(rect, GFC_COLOR_LIGHTGREEN);
			
		} else if (curr->type == ST_CIRCLE) {
			// TODO implement drawing the circle
			// This should really be broken out into its own function
		} else {
			// TODO implement drawing the line
			// This should really be broken out into its own function
		}

	}
}

/**
 * @brief check if an entity is overlapping with any static shape in the space
 * @param entity the entity whose bounds are being checked with static shapes in the world
 * @return a list of shape overlaps as Vector2Ds
 * @note this list is not freed on its own, and must be freed by the function caller
 */
GFC_List *space_overlap_entity_static_shape(Space *self, Entity *entity) {
	Uint32 i, c;
	GFC_Shape *curr;
	GFC_Vector2D poc;
	GFC_Vector2D normal;

	// Create the collision list
	GFC_List *collision_list = gfc_list_new();

	// Iterate through all shapes in the space
	c = gfc_list_count(self->static_shapes);

	// Entity world space collider
	GFC_Circle world_space_collider = gfc_circle(entity->collider.x + entity->position.x, entity->collider.y + entity->position.y, entity->collider.r);

	// For each static body do the overlap test
	for (i = 0; i < c; ++i) {
		curr = gfc_list_get_nth(self->static_shapes, i);

		if (gfc_shape_overlap_poc(*curr, gfc_shape_from_circle(world_space_collider), &poc, &normal)) {
			Collision *coll = collision_new();
			gfc_vector2d_copy(coll->poc, poc);
			gfc_vector2d_copy(coll->normal, normal);
			gfc_list_append(collision_list, coll);
		}
	}

	if (!gfc_list_count(collision_list)) {
		gfc_list_delete(collision_list);
		return NULL;
	} else {
		return collision_list;
	}
}

void space_add_entity(Space *self, Entity *ent) {
	slog("entering the function");
	if (!ent || !ent->body || !self || !self->bodies) return;
	slog("succ2?");
	// Append the entity's body to the space
	gfc_list_append(self->bodies, ent->body);
}

/**
 * @brief take a simulation step
 * @param self the space object to be stepped
 * @param delta_time the time that passes in a single step
 */
void space_step(Space *self, float delta_time) {
	int i, c;
	Body *curr;
	GFC_Vector2D dx, dv;
	slog("step");

	c = gfc_list_count(self->bodies);
	for (i = 0; i < c; ++i) {
		// Get and verify ptr
		curr = gfc_list_get_nth(self->bodies, i);
		if (!curr) continue;

		// Integrate forces (for now just add net_acceleration to acceleration)
		curr->net_acceleration = gfc_vector2d(0, 0); // Reset net acceleration
		gfc_vector2d_copy(curr->net_acceleration, curr->acceleration); // Copy applied acceleration into net acceleration
									       // Apply other forces (joints, e.t.c.)

		// Semi implicit euler method
		// Integrate velocity
		gfc_vector2d_copy(dv, curr->net_acceleration);
		gfc_vector2d_scale_by(dv, dv, gfc_vector2d(delta_time, delta_time));
		gfc_vector2d_add(curr->velocity, curr->velocity, dv);
		slog("%f, %f", curr->velocity.x, curr->velocity.y);
		
		// Then integrate position
		gfc_vector2d_copy(dx, curr->velocity);
		gfc_vector2d_scale_by(dx, dx, gfc_vector2d(delta_time, delta_time));
		gfc_vector2d_add(curr->position, curr->position, dx);
	}
}

/**
 * @brief update the physics space
 * @param self the space object to be updated
 */
void space_update(Space *self) {
	if (!self) return;

	for (int i = 0; i < 10; i++) {
		space_step(self, 0.1);
	}
}


