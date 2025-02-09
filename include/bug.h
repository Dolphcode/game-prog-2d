#ifndef __BUG_H__
#define __BUG_H__

#include "entity.h"

/**
 * @brief spawn a new bug entity
 * @param position where to spawn it
 * @return NULL on error
 */
Entity *bug_new_entity(GFC_Vector2D position);

#endif
