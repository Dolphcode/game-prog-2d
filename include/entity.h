#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "simple_json.h"

#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_list.h"

#include "gf2d_sprite.h"

// Debug constants
extern Uint8 	DRAW_CENTER; // <Draw the center points of entities
extern Uint8	DRAW_BOUNDS; // <Draw the bounds of entities

/** A description of how the entity will fit into the overall gameplay loop.
 *  The entity object is both a data container and a driver for how the entity behaves
 *  The gameplay loop will always consist of two states
 *  1. The current frame
 *  2. The next physics state
 *
 *  The entity object is designed to be an interface between the player and game logic, and the physics engine
 *
 *  The think() function is responsible for modifying the game state based on the current frame. It will always
 *  happen before the physics engine processes the future state as the physics engine will determine the next frame based
 *  on modifications to the game state between think() and update(). think() serves as a hook for modifications we want
 *  to make to the game state before the physics engine executes its time step
 *   - think() is useful for driving the entity based on the current game state (modifying velocity and acceleration)
 *   - any operations that are dependent on the state of other entities should occur in think()
 *   - think() should not be used for modifying state that could affect the think() function of other entities
 *   	- position should not be modified in think
 *   	- collider info should not be modified in think?
 *  	- timers should not be updated in think() for best practice, as timers timing out can affect game state?
 *   - in general other entities don't care about what an entity *wants* to do, only what it *is* doing. If you refrain
 *     from modifying what an entity is currently doing in think() you'll be generally okay
 *
 *  The update() function is responsible for a few things
 *  1. Interpolating between the current frame and the next fully processed frame processed by the physics engine in order
 *     to *advance* the current frame. After update() the current frame will have been shifted into the next frame for
 *     think() to read.
 *  2. Updating the entity's own game state for the current frame before the next think() call
 *
 *  Just remember that the call order is always
 *  1. think() - modify state based on the current frame
 *  2. physics_update() - Sync think() modifications with physics body, time step and resolve collisions, then produce next position for entity
 *  3. update() - advance the current frame to the next one/advance towards the current state calculated by the physics engine
 */
typedef struct Entity_S
{
	// Entity Metadata
	GFC_TextLine	name;		// <The name of the entity object for debugging purposes
	Uint8		_inuse;		// <Whether the entity is in use or not (for managing memory)
	float		lifetime;	// <How long the entity has been alive for

	// Entity Graphical Information
	Sprite		*sprite;	// <The entity's corresponding sprite/graphical representation
	GFC_Vector2D	sprite_offset;	// <Where the entity point is relative to the top left corner of the sprite
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
 * @brief free entities in the entity system from a list
 * @param entity_list the list of data to be freed
 */
void entity_system_free_list(GFC_List *entity_list);

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
 * @param self the reference to the entity object
 */
void entity_free(Entity *);

/**
 * @brief the default draw function for entities (called if an entity doesn't have a specified draw function)
 * @param self the reference to the entity object
 */
void entity_draw(Entity *);

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
