#include "simple_logger.h"

#include "gfc_list.h"

#include "entity.h"

typedef struct
{
	Uint32	entity_max;
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

void entity_system_draw_all() {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		// Check if the entity slot we're looking at is inuse and has a draw function
		if (entity_system.entity_list[i]._inuse 
				&& entity_system.entity_list[i].draw) {
			entity_system.entity_list[i].draw(&entity_system.entity_list[i]);
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

	// Mark entity as no longer in use
	ent->_inuse = 0;
}
