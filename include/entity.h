#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "simple_json.h"

#include "gfc_text.h"
#include "gfc_vector.h"

#include "gf2d_sprite.h"

typedef struct Entity_S
{
	// Entity Metadata
	GFC_TextLine	name;		// <The name of the entity object for debugging purposes
	Uint8		_inuse;		// <Whether the entity is in use or not (for managing memory)
	float		lifetime;	// <How long the entity has been alive for

	// Entity Graphical Information
	Sprite		*sprite;	// <The entity's corresponding sprite/graphical representation
	float		frame;		// <The current frame of the entity's sprite animation

	// Physics Quantities
	GFC_Vector2D	position;	// <The entity's position in global space
	GFC_Vector2D	velocity;	// <The entity's velocity for physics calculations
	GFC_Vector2D	acceleration;	// <The entity's acceleration for physics calculations

	// Functions
	void		(*think)(struct Entity_S *self);	// <Called before update(), used to determine entity actions
	void		(*update)(struct Entity_S *self);	// <Called after think(), used to update entity state
	void		(*draw)(struct Entity_S *self);		// <Called after update(), draw the entity (along with any other necessary draw calls)

}Entity;

/**
 * @brief initialize the entity list and manager
 * @param maxEnts upper limit for how many entities can exist at one time
 */
void entity_system_init(Uint32 maxEnts);

/**
 * @brief free all entities in the entity system
 */
void entity_system_free_all();

/**
 * @brief draw all entities
 */
void entity_system_draw_all();

/**
 * @brief call the think() function of all entities to evaluate the current game state
 */
void entity_system_think_all();

/**
 * @brief call the update() function of all entities
 */
void entity_system_update_all();

/**
 * @brief get a new empty entity object
 * @return NULL if out of entity slots or a blank Entity object otherwise
 */
Entity* entity_new();

/**
 * @brief free a previously created entity
 */
void entity_free(Entity *);

/**
 * @brief configures an entity from a def file (given via filepath)
 * @param self the entity pointer whose data should be populated
 * @param filename the path to the def file being loaded
 */
void entity_configure_from_file(Entity *self, const char *filename);

/**
 * @brief configures an entity from a json object
 * @param self the entity pointer whose data should be populated
 * @param json the json object containing the data to be copied
 */
void entity_configure(Entity *self, SJson *json);

#endif
