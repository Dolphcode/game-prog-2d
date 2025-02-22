#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_input.h"
#include "gfc_color.h"
#include "gfc_list.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "gf2d_draw.h"

#include "player.h"
#include "bug.h"
#include "camera.h"
#include "world.h"
#include "collision.h"
#include "space.h"

static float projv1 = 2;
static float projv2 = 1;

GFC_List *player_collision_list = NULL;

void player_update(Entity *self) {
	if (!self) return;
	
	// zero velocity
	self->velocity = gfc_vector2d(0, 0);
	
	// check input
	if (gfc_input_command_down("left")) {
		self->velocity.x -= 1;
	}
	if (gfc_input_command_down("right")) {
		self->velocity.x += 1;
	}
	if (gfc_input_command_down("up")) {
		self->velocity.y -= 1;
	}
	if (gfc_input_command_down("down")) {
		self->velocity.y += 1;
	}

	if (gfc_input_command_pressed("shoot1")) {
		Entity *bug = bug_new_entity(self->position, "./def/bugs/bug1.def");
		bug->velocity = gfc_vector2d(projv1, 0);
	}

	if (gfc_input_command_pressed("shoot2")) {
		Entity *bug = bug_new_entity(self->position, "./def/bugs/bug2.def");
		bug->velocity = gfc_vector2d(0, projv2);
	}
	
	gfc_vector2d_normalize(&self->velocity);
	gfc_vector2d_scale_by(self->velocity, self->velocity, gfc_vector2d(5, 5));

	gfc_vector2d_add(self->position, self->position, self->velocity);

	GFC_List *collision_list = space_overlap_entity_static_shape(world_get_active()->space, self);
	if (collision_list) {
		int i, c;
		c = gfc_list_count(collision_list);
		for (i = 0; i < c; ++i) {
			Collision *c = ((Collision*)gfc_list_get_nth(collision_list, i));
			slog("Point of collision %i: (%f, %f)", i, c->poc.x, c->poc.y);
		}
		
		// Reassign for draw call
		if (DRAW_COLLISIONS) {
			player_collision_list = collision_list;
		}
	}
}

void player_draw(Entity *self) {
	if (!self) return;

	// Draw the player normally
	entity_draw(self);

	// Now draw collision points
	if (player_collision_list) {
		int i, c;
		Collision *curr;
		GFC_Vector2D drawpos, normalendpos, normalenddrawpos;
		c = gfc_list_count(player_collision_list);

		for (i = 0; i < c; ++i) {
			curr = gfc_list_get_nth(player_collision_list, i);
			if (!curr) continue; // Validate pointer
			drawpos = main_camera_calc_drawpos(curr->poc);
			gfc_vector2d_scale_by(normalendpos, curr->normal, gfc_vector2d(10, 10));
			gfc_vector2d_add(normalenddrawpos, curr->poc, normalendpos);
			normalenddrawpos = main_camera_calc_drawpos(normalenddrawpos);
			gf2d_draw_circle(drawpos, 4, GFC_COLOR_BLUE);
			gf2d_draw_line(drawpos, normalenddrawpos, GFC_COLOR_BLUE);
			slog("drawn");
		}
		

		collision_list_free(player_collision_list);
		player_collision_list = NULL;
	}	
}

Entity *player_new_entity(GFC_Vector2D position) {
	Entity *self;
	self = entity_new();
	if (!self) {
		slog("failed to spawn a new player entity");
		return NULL;
	}

	// Copy position date into player
	gfc_vector2d_copy(self->position, position);

	// Initialize player entity from config
	entity_configure_from_file(self, "./def/player.def");
	
	// Assign player functions
	self->update = player_update;
	self->draw = player_draw;

	return self;
}

