#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_list.h"
#include "gfc_config.h"

#include "gf2d_graphics.h"
#include "gf2d_draw.h"

#include "entity.h"
#include "camera.h"
#include "world.h"
#include "physicsbody.h"

Uint8	DRAW_CENTER = 0;
Uint8	DRAW_BOUNDS = 0;

typedef struct
{
	Uint32	entity_max;
	Uint32	active_entities;
	Entity	*entity_list;
}EntitySystem;

static EntitySystem entity_system = {0};

/**
 * @brief frees all entities and closes the entity system
 */
void entity_system_close() {
	if (entity_system.entity_list) {
		entity_system_free_all();
		free(entity_system.entity_list);
		entity_system.entity_list = NULL; // Reset entity system object completely
	}
	slog("entity system closed successfully");
}

void entity_system_init(Uint32 max_ents) {
	// Make sure max_ents is nonzero
	if (!max_ents) {
		slog("cannot initialize entity list with 0 entities");
		return;
	}
	
	// Initialize the entity system with list and max ents
	entity_system.entity_list = gfc_allocate_array(sizeof(Entity), max_ents);
	if (!entity_system.entity_list) {
		slog("failed to allocate %i entities", max_ents);
		return;
	}
	entity_system.entity_max = max_ents;

	// Queue entity system for closing
	atexit(entity_system_close);
	slog("entity list initialized successfully");
}

void entity_system_free_all() {
	// Mark all entities as no longer in use
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (entity_system.entity_list[i]._inuse) {
			entity_free(&entity_system.entity_list[i]);
		}
	}
}

void entity_system_free_list(GFC_List *entity_list) {
	int i, c = gfc_list_count(entity_list);

	// Mark entities in this list as no longer in use
	for (i = 0; i < c; i++) {
		if (((Entity*)gfc_list_get_nth(entity_list, i))->_inuse) {
			entity_free((Entity*)gfc_list_get_nth(entity_list, i));
		}
	}
}

void entity_system_think_all() {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		// Check if the entity slot we're looking at is inuse and has a think function
		if (entity_system.entity_list[i]._inuse 
				&& entity_system.entity_list[i].think) {
			entity_system.entity_list[i].think(&entity_system.entity_list[i]);
		}
	}
}

void entity_system_update_all() {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		// Check if the entity slot we're looking at is inuse and has an update function
		if (entity_system.entity_list[i]._inuse 
				&& entity_system.entity_list[i].update) {
			entity_system.entity_list[i].update(&entity_system.entity_list[i]);
		}
	}
	// slog("Active entities: %i", entity_system.active_entities); TODO: Make this a UI option later
}

void entity_system_presync_all() {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		// Check if the entity slot we're looking at is inuse and has a physics body
		if (entity_system.entity_list[i]._inuse 
				&& entity_system.entity_list[i].body) {
			gfc_vector2d_copy((&entity_system.entity_list[i])->body->position, (&entity_system.entity_list[i])->position);
			gfc_vector2d_copy((&entity_system.entity_list[i])->body->velocity, (&entity_system.entity_list[i])->velocity);
			gfc_vector2d_copy((&entity_system.entity_list[i])->body->acceleration, (&entity_system.entity_list[i])->acceleration);
			(&entity_system.entity_list[i])->body->grounded = 0; // Reset grounded state, redetermine if entity is grounded

		}
	}
	// slog("Active entities: %i", entity_system.active_entities); TODO: Make this a UI option later
}

void entity_system_postsync_all() {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		// Check if the entity slot we're looking at is inuse and has a physics body
		if (entity_system.entity_list[i]._inuse 
				&& entity_system.entity_list[i].body) {
			gfc_vector2d_copy((&entity_system.entity_list[i])->position, (&entity_system.entity_list[i])->body->position);
			gfc_vector2d_copy((&entity_system.entity_list[i])->velocity, (&entity_system.entity_list[i])->body->velocity);
			gfc_vector2d_copy((&entity_system.entity_list[i])->acceleration, (&entity_system.entity_list[i])->body->acceleration);

		}
	}
	// slog("Active entities: %i", entity_system.active_entities); TODO: Make this a UI option later
}

void entity_system_draw_all() {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		// Check if the entity slot we're looking at is inuse and has a draw function
		if (entity_system.entity_list[i]._inuse) { 
			if ( entity_system.entity_list[i].draw) {
				entity_system.entity_list[i].draw(&entity_system.entity_list[i]);
			} else {
				entity_draw(&entity_system.entity_list[i]);
			}
		}
	}
}

Entity* entity_new() {
	// We need to search for an open slot first
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (entity_system.entity_list[i]._inuse) continue; // Skip active entities
		memset(&entity_system.entity_list[i], 0, sizeof(Entity));

		entity_system.entity_list[i]._inuse = 1;
		entity_system.active_entities++;
		return &entity_system.entity_list[i];
	}
	// Fail if no slot could be found
	return NULL;
}

void entity_free(Entity *ent) {
	// If the pointer is invalid no entity to free here, just fail
	if (!ent) return;

	// Free the sprite if need be
	if (ent->sprite) gf2d_sprite_free(ent->sprite);

	// Free the physics body if we have one
	if (ent->body) {
		space_remove_entity(world_get_active()->space, ent);
		physics_body_free(ent->body);
		ent->body = NULL;
	}

	// Mark entity as no longer in use
	ent->_inuse = 0;
	entity_system.active_entities--;
}

void entity_draw(Entity *self) {
	// Verify pointers
	if (!self || !self->sprite) return;

	// Get a pointer to the main camera
	Camera* main_camera = camera_get_main();

	// Calculate draw position and scale
	GFC_Vector2D scale = main_camera_get_zoom();
	
	/*
	GFC_Vector2D draw_pos = {0};
	gfc_vector2d_add(draw_pos, self->position, main_camera_get_offset());

	gfc_vector2d_scale_by(draw_pos, draw_pos, scale);

	GFC_Vector2D screen_res = gf2d_graphics_get_resolution();
	gfc_vector2d_scale_by(screen_res, screen_res, gfc_vector2d(0.5, 0.5));
	gfc_vector2d_add(draw_pos, draw_pos, screen_res);
	*/
	GFC_Vector2D draw_pos = main_camera_calc_drawpos(self->position);

	GFC_Vector2D center = self->sprite_offset;

	// Draw the sprite
	gf2d_sprite_draw(
		self->sprite,
		draw_pos,
		&scale,
		&center,
		NULL,
		NULL,
		NULL,
		(Uint32)self->frame);

	// Draw the point
	if (DRAW_CENTER) gf2d_draw_circle(draw_pos, 4, GFC_COLOR_LIGHTGREEN);
}	

/**
 * @brief load the entity body configuration from a def file
 * @param self the entity being configured
 * @param json the json object which data is being loaded from for the collider
 * @param hitbox_json the json obejct which hitbox data is being loaded from
 */
void entity_configure_body(Entity *self, SJson *json, SJson *hitbox_json) {
	if (!self) return;

	// Create the physics body
	self->body = physics_body_new(GLOBAL_MAX_COLLISIONS);

	// Validate the pointer
	if (!self->body) return;

	if (json) {
		// Load the collider information
		GFC_Vector2D offset, bounds;
		Uint8 can_collide;
       		sj_object_get_vector2d(json, "colliderOffset", &offset);
		sj_object_get_vector2d(json, "colliderSize", &bounds);
		sj_object_get_uint8(json, "canCollide", &can_collide);

		// Assign collider info
		self->body->collider.x = offset.x;
		self->body->collider.y = offset.y;
		self->body->collider.w = bounds.x;
		self->body->collider.h = bounds.y;
		self->body->can_collide = can_collide;
	}

	if (hitbox_json) {
		GFC_Vector2D offset;
		float radius;
		sj_object_get_vector2d(hitbox_json, "hitboxOffset", &offset);
		sj_object_get_float(hitbox_json, "hitboxRadius", &radius);

		self->body->hitbox = gfc_shape_from_circle(gfc_circle(offset.x, offset.y, radius));
	}
	self->body->ent = self;
}

void entity_configure_from_file(Entity *self, const char *filename) {
	if (!filename) return;
	SJson *json = sj_load(filename);
	entity_configure(self, json);
	sj_free(json);
}

void entity_configure(Entity *self, SJson *json) {
	const char *sprite = NULL;
	if ((!self)||(!json)) return;

	sprite = sj_object_get_string(json, "sprite");
	if (sprite) {
		GFC_Vector2D frame_size = {0};
		Uint32 fpl = 0;
		sj_object_get_vector2d(json, "spriteSize", &frame_size);
		sj_object_get_uint32(json, "spriteFPL", &fpl);
		self->sprite = gf2d_sprite_load_all(
			sprite,
			(Uint32)frame_size.x,
			(Uint32)frame_size.y,
			fpl,
			0);

		GFC_Vector2D sprite_offset = {0};
		sj_object_get_vector2d(json, "spriteOffset", &sprite_offset);
		self->sprite_offset = sprite_offset;
	}
	// Get the collider
	SJson *coll_info = sj_object_get_value(json, "collider");
	SJson *hitbox_info = sj_object_get_value(json, "hitbox");
	entity_configure_body(self, coll_info, hitbox_info);
	space_add_entity(world_get_active()->space, self);	

	// Load the entity name
	const char *name = NULL;
	name = sj_object_get_string(json, "name");
	if (sprite) gfc_line_cpy(self->name, name);
}
