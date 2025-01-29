#include "simple_logger.h"

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gf2d_sprite.h"

#include "entity.h"

typedef struct
{
	Uint32 	entity_max;
	Entity 	*entity_list;
}EntitySystem;

static EntitySystem entity_system = {0};

void entity_draw(Entity*);

void entity_system_close() {
	if (entity_system.entity_list) {
		entity_system_free_all();
		free(entity_system.entity_list);
		entity_system.entity_list = NULL;
	}
	slog("entity system closed successfully");
}

void entity_system_init(Uint32 maxEnts) {
	if (!maxEnts) {
		slog("cannot initialize entity list with 0 entity max");
		return;
	}
	entity_system.entity_list = gfc_allocate_array(sizeof(Entity),maxEnts);
	if (!entity_system.entity_list) {
		slog("failed to allocate %i entities", maxEnts);
		return;
	}
	entity_system.entity_max = maxEnts;
	atexit(entity_system_close);
	slog("entity list initialized successfully");
}

void entity_system_free_all() {
	int i;
	for (i = 0; i < entity_system.entity_max;i++) {
		if (entity_system.entity_list[i]._inuse) {
			entity_free(&entity_system.entity_list[i]);
		}
	}
}

void entity_system_draw_all() {
	int i;
	for (i = 0; i < entity_system.entity_max;i++) {
		if (entity_system.entity_list[i]._inuse) {
	       		entity_draw(&entity_system.entity_list[i]);

			// Testing increasing frame number
			entity_system.entity_list[i].frame += 0.1;
			if (entity_system.entity_list[i].frame >= 16.0) {
				entity_system.entity_list[i].frame = 0.0;
			}
		}
	}	       
}

Entity *entity_new() {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (entity_system.entity_list[i]._inuse)continue; // Skip active entities
		memset(&entity_system.entity_list[i],0,sizeof(Entity));
		entity_system.entity_list[i]._inuse = 1;
		return &entity_system.entity_list[i];
	}
	return NULL;
}

void entity_free(Entity *ent) {
	// If the pointer is invalid no entity to free here, just fail
	if (!ent) {
		return;
	}

	if (ent->sprite) {
		gf2d_sprite_free(ent->sprite);
	}
}

void entity_draw(Entity *self){
	if (!self) return;
	if (!self->sprite) return;
	gf2d_sprite_draw(
		self->sprite,
		self->position,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		(Uint32)self->frame);
}
