#include "simple_logger.h"

#include "world.h"

World *world_load(const char *filename) {
	return NULL;
}

void world_free(World *world) {
	// Check pointer
	if (!world) return;
	
	// Free the sprites
	if (world->background) gf2d_sprite_free(world->background);
	if (world->foreground) gf2d_sprite_free(world->foreground);

	// Free the entities in the entity list
	int i,c;
	Entity *ent;
	if (world->entity_list) {
		c = gfc_list_count(world->entity_list);
		for (i = 0; i < c; i++) {
			ent = gfc_list_nth(world->entity_list, i);
			if (!ent) continue;
			entity_free(ent);
		}
		gfc_list_delete(world->entity_list);
	}
	
	// Free the camera object
	if (world->main_camera) camera_free(world->main_camera);
}

World *world_new() {
	World *world;
	world = gfc_allocate_array(sizeof(World), 1);
	if (!world) {
		slog("failed to allocate world");
		return NULL;
	}

	return world;
}

void world_draw() {
	// Draw the background first based on the camera position
}
