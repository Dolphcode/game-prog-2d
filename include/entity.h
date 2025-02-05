#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "simple_json.h"

#include "gfc_text.h"
#include "gfc_list.h"
#include "gfc_vector.h"
#include "gf2d_sprite.h"

typedef struct Entity_S
{
	// Entity Metadata
	GFC_TextLine	name;		/**<entity name for debugging purposes*/
	Uint8		_inuse;		/**<memory management flag*/

	// Entity Graphical Information
	Sprite		*sprite;	/**<entity sprite*/
	float frame;

	// Physics quantities
	GFC_Vector2D	position;	/**<entity position*/
	GFC_Vector2D	velocity;	/**<entity velocity vector*/

	// Function
	void		(*think)(struct Entity_S *self);	/**<pointer to a think function*/
}Entity;

/**
 * @brief initialize the entity sub entity_system_init
 * @param maxEnts upper limit for how many entities can exist at once
 */
void entity_system_init(Uint32 maxEnts);

/**
 * @brief free all entities in the manager
 */
void entity_system_free_all();

/**
 * @brief draw all entities
 */
void entity_system_draw_all();

/**
 * @brief draw all entities in the provided list
 * @param entities a GFC_List of entity pointers
 */
void entity_system_draw_list(GFC_List *entities);

/**
 * @brief make all entities think
 */
void entity_system_think_all();

/**
 * @brief get a new empty entity to work with
 * @return NULL if out of entities, or a blank entity otherwise
 */
Entity* entity_new();

/**
 * @brief free a previously created entity
 */
void entity_free(Entity *);

/**
 * @brief configures an entity from a def file given a filenamei
 * @param self the entity pointer
 * @param filename the path to the def file being loaded
 */
void entity_configure_from_file(Entity *self, const char *filename);

/**
 * @brief configures an entity from a def file
 * @param self the entity pointer
 * @param json the json object containing the data to be loaded
 */
void entity_configure(Entity *self, SJson *json);
#endif
