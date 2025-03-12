#ifndef __SPAWN_H__
#define __SPAWN_H__

#include "simple_json.h"

#include "gfc_vector.h"

#include "entity.h"

typedef struct {
	const char *name;
	const char *config;
	Entity *(*spawn)(GFC_Vector2D pos, const char *config);
}Spawn;

/**
 * @brief Spawn an entity from the spawn list, referring to it by name, at a given position
 * @param name the name/identifier for the entity to be spawned
 * @param position where in the world should this entity be spawned
 * @param config a path to a config file which may be used to configure the spawned entity
 */
Entity* spawn_entity(const char *name, GFC_Vector2D position, const char *config);

Entity* spawn_entity_default(const char *name, GFC_Vector2D position);
#endif
