#include "simple_logger.h"

#include "gfc_color.h"

#include "gf2d_draw.h"

#include "camera.h"
#include "physicsbody.h"
#include "space.h"

/**
 * @brief evaluate overlaps in the space
 * @param self the space we're testing overlaps for
 * @param body the body that's being tested
 */
void space_body_static_overlaps(Space *self, PhysicsBody *body);

/**
 * @brief resolve collisions
 * @param self the space we're resolving overlaps for
 * @param body the body whose overlaps are being resolved
 */
void space_body_resolve_overlaps(Space *self, PhysicsBody *body);

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

		// Resolve overlaps
		space_body_resolve_overlaps(self, curr);
	}
}

void space_update(Space *self) {
	int i;
	for (i = 0; i < 10; ++i) {
	       space_step(self, 0.1 * (1.0/60.0));
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

// Collision testing and resolution

void space_body_resolve_overlaps(Space *self, PhysicsBody *body) {
	if (!self || !self->static_bodies || !body || !body->max_collisions) return;
	Collision *curr, *max = NULL;
	int j, count;
	int i = 0;
	space_body_static_overlaps(self, body);
	while (body->unresolved_collisions && i < 5) { // Perform collision resolved 5x max
		// Find the collision with the maximum overlap and resolve that one first
		count = body->unresolved_collisions;
		for (j = 0; j < count; j++) {
			curr = &body->collision_list[j];
			if (!curr) continue;

			if (!max || max->overlap < curr->overlap) {
				max = curr;
			}
		}


		// Now resolve the collision based on normal
		if (max->normal.x < 0) {
			body->position.x = max->shape.x - body->collider.w - body->collider.x;
		} else if (max->normal.x > 0)  {
			body->position.x = max->shape.x + max->shape.w - body->collider.x;
		}

		if (max->normal.y < 0) {
			body->position.y = max->shape.y - body->collider.h - body->collider.y;
		} else if (max->normal.y > 0) {
			body->position.y = max->shape.y + max->shape.h - body->collider.y;
		}

		// Recalculate overlaps (maybe optimize later but probably fine for now)
		space_body_static_overlaps(self, body);
		++i;
	}

}

void space_body_static_overlaps(Space *self, PhysicsBody *body) {
	if (!self || !self->static_bodies || !body || !body->max_collisions) return;
	int i, count;
	GFC_Rect *curr;
	GFC_Rect world_body;
	Collision *coll;
	float min_edge, max_edge, x_overlap, y_overlap;
	float lower_corr, upper_corr, x_corr, y_corr, x_norm, y_norm;
	
	// Reset collisions
	count = body->max_collisions;
	memset(&body->collision_list[0], 0, count * sizeof(Collision));
	body->unresolved_collisions = 0;

	// Compute world body position
	world_body = gfc_rect(body->collider.x + body->position.x, body->collider.y + body->position.y, body->collider.w, body->collider.h);

	// Evaluate collisions
	count = gfc_list_count(self->static_bodies);
	for (i = 0; i < count; ++i) {
		curr = gfc_list_get_nth(self->static_bodies, i);
		if (!curr) continue;
		
		if (gfc_rect_overlap(*curr, world_body)) {
			// Get the collision object
			coll = &body->collision_list[body->unresolved_collisions];

			// Increment unresolved collisions if possible
			if (!coll) continue;
			body->unresolved_collisions++;
			
			// Now set compute overlaps
			if (world_body.x > curr->x) {
				min_edge = world_body.x;
			} else {
				min_edge = curr->x;
			}

			if (world_body.x + world_body.w < curr->x + curr->w) {
				max_edge = world_body.x + world_body.w;
			} else {
				max_edge = curr->x + curr->w;
			}
			x_overlap = max_edge - min_edge;

			if (world_body.y > curr->y) {
				min_edge = world_body.y;
			} else {
				min_edge = curr->y;
			}

			if (world_body.y + world_body.h < curr->y + curr->h) {
				max_edge = world_body.y + world_body.h;
			} else {
				max_edge = curr->y + curr->h;
			}
			y_overlap = max_edge - min_edge;

			coll->overlap = x_overlap * y_overlap;

			// Now compute normals
			lower_corr = world_body.x - (curr->x - world_body.w);
			upper_corr = (curr->x + curr->w) - world_body.x;
			if (lower_corr < upper_corr) {
				x_norm = -1;
				x_corr = lower_corr;
			} else {
				x_norm = 1;
				x_corr = upper_corr;
			}
			
			lower_corr = world_body.y - (curr->y - world_body.h);
			upper_corr = (curr->y + curr->h) - world_body.y;
			if (lower_corr < upper_corr) {
				y_norm = -1;
				y_corr = lower_corr;
			} else {
				y_norm = 1;
				y_corr = upper_corr;
			}

			// Determine which way is easier to correct
			if (x_corr < y_corr) {
				coll->normal.x = x_norm;
				coll->normal.y = 0;
			} else if (x_corr > y_corr) {
				coll->normal.y = y_norm;
				coll->normal.x = 0;
			} else { // Equivalent case, collide corner
				coll->normal.x = x_norm;
				coll->normal.y = y_norm;
				gfc_vector2d_normalize(&coll->normal);
			}

			// And copy shape information
			coll->shape.x = curr->x;
			coll->shape.y = curr->y;
			coll->shape.w = curr->w;
			coll->shape.h = curr->h;
		}
	}
}
