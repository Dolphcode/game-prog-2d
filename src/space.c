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

	return space;
}

void space_free(Space *self) {
	// Free the list of static shapes
	Uint32 i, list_count;
	list_count = gfc_list_get_count(self->static_shapes);
	for (i = 0; i < list_count; ++i) {
		free(gfc_list_get_nth(self->static_shapes, i));
	}

	gfc_list_delete(self->static_shapes);

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

