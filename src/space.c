#include "simple_logger.h"

#include "gfc_color.h"

#include "gf2d_draw.h"

#include "camera.h"
#include "physicsbody.h"
#include "space.h"

Space *space_new() {
    // First create the space object
    Space* space = gfc_allocate_array(sizeof(Space), 1);
    if (!space) {
        slog("failed to allocate memory for space");
        return NULL;
    }

    // Create the static and physics body lists
    space->static_bodies = gfc_list_new();
    space->physics_bodies = gfc_list_new();

    // Verify and output warnings
    if (!space->static_bodies) slog("WARNING: Could not allocate memory for static bodies list");
    if (!space->physics_bodies) slog("WARNING: Could not allocate memory for physics bodies list");

    return space;
}

void space_free(Space *self) {
    if (!self) return;
    int i, count;

    // Free the static bodies
    if (self->static_bodies) {
        count = gfc_list_count(self->static_bodies);
        for (i = count - 1; i >= 0; --i) {
            // Free the object
            free(gfc_list_get_nth(self->static_bodies, i));

            // Free the slot
            gfc_list_delete_nth(self->static_bodies, i);
        }
        // Delete the list
        gfc_list_delete(self->static_bodies);
    }

    // Free the physics bodies
    if (self->physics_bodies) {
        count = gfc_list_count(self->physics_bodies);
        for (i = count - 1; i >= 0; --i) {
            // Free the object (nope entity is in charge)
            //physics_body_free(gfc_list_get_nth(self->physics_bodies, i));

            // Free the slot
            gfc_list_delete_nth(self->physics_bodies, i);
        }
        // Delete the list
        gfc_list_delete(self->physics_bodies);
    }

    // Free the space object
    free(self);
}

void space_add_static_rect(Space *self, GFC_Rect shape) {
	// Validate pointer
	if (!self || !self->static_bodies) return;

	// Create the rect memory slot
	GFC_Rect *rect = gfc_allocate_array(sizeof(GFC_Rect), 1);
	if (!rect) {
		slog("failed to create memory slot for static shape");
		return;
	}

	// Copy data into the rect
	rect->x = shape.x;
	rect->y = shape.y;
	rect->w = shape.w;
	rect->h = shape.h;

	// Append it to the list of static bodies
	gfc_list_append(self->static_bodies, rect);
}

void space_add_entity(Space *self, Entity *ent) {
	if (!self || !self->physics_bodies || !ent || !ent->body) return;
	gfc_list_append(self->physics_bodies, ent->body);
}


void space_step(Space *self, float delta_time) {
	int i, count;
	PhysicsBody *curr;
	GFC_Vector2D dv, dx;

	count = gfc_list_count(self->physics_bodies);
	for (i = 0; i < count; ++i) {
		// Get the current reference and validate
		curr = gfc_list_get_nth(self->physics_bodies, i);
		if (!curr) continue;
		
		// Reset acceleration and calculate forces
		curr->net_acceleration = gfc_vector2d(0, 0);
		gfc_vector2d_add(curr->net_acceleration, curr->net_acceleration, curr->acceleration);

		// Semi implicit euler integration
		// Integrate velocity first
		gfc_vector2d_copy(dv, curr->net_acceleration);
		gfc_vector2d_scale_by(dv, dv, gfc_vector2d(delta_time, delta_time));
		gfc_vector2d_add(curr->velocity, curr->velocity, dv);

		// Then integrate position
		gfc_vector2d_copy(dx, curr->velocity);
		gfc_vector2d_scale_by(dx, dx, gfc_vector2d(delta_time, delta_time));
		gfc_vector2d_add(curr->position, curr->position, dx);
	}
}

void space_update(Space *self) {
	int i;
	for (i = 0; i < 10; ++i) {
	       space_step(self, 0.1);
	}	       
}

void space_draw(Space *self) {
	if (!self) return;
	int i, count;
	GFC_Rect *curr;
	PhysicsBody *curr_body;
	GFC_Rect drawable;
	GFC_Vector2D drawpos;

	// Draw the static bodies
	if (self->static_bodies) {
		count = gfc_list_count(self->static_bodies);
		for (i = 0; i < count; ++i) {
			curr = gfc_list_get_nth(self->static_bodies, i);
			if (!curr) continue;

			// Generate the drawable rect
			drawpos.x = curr->x;
			drawpos.y = curr->y;
			drawpos = main_camera_calc_drawpos(drawpos);
			drawable.x = drawpos.x;
			drawable.y = drawpos.y;
			drawable.w = curr->w;
			drawable.h = curr->h;

			// Draw the rect in orange
			gf2d_draw_rect(drawable, GFC_COLOR_ORANGE);
		}
	}

	// Draw the physics bodies
	if (self->physics_bodies) {
		count = gfc_list_count(self->physics_bodies);
		for (i = 0; i < count; ++i) {
			curr_body = gfc_list_get_nth(self->physics_bodies, i);
			if (!curr) continue;

			// Generate the drawable rect
			drawpos.x = curr_body->position.x + curr_body->collider.x;
			drawpos.y = curr_body->position.y + curr_body->collider.y;
			drawpos = main_camera_calc_drawpos(drawpos);
			drawable.x = drawpos.x;
			drawable.y = drawpos.y;
			drawable.w = curr_body->collider.w;
			drawable.h = curr_body->collider.h;

			// Draw the rect in orange
			gf2d_draw_rect(drawable, GFC_COLOR_RED);
		}
	}
}
