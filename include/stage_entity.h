#ifndef __STAGE_ENTITY_H__
#define __STAGE_ENTITY_H__

#include "gfc_vector.h"

#include "entity.h"

/**
 * @brief spawns a temporary platform in the world
 * @param position the position of the top left corner of the temporary platform
 * @config the configuration file of the temporary platform
 */
Entity* spawn_temp_platform(GFC_Vector2D position, const char *config);

/**
 * @brief spawns a buzzsaw in the world (stage hazard)
 * @param position the position of the top left corner of the buzzsaw
 * @config the configuration file of the buzzsaw
 */
Entity* spawn_buzzsaw(GFC_Vector2D position, const char *config);

Entity* spawn_medkit(GFC_Vector2D position, const char *config);
/**
 * @brief spawns a turret into the world
 * @param position where the turret should be spawned
 * @config the configuration file of the turret
 */
Entity* spawn_turret(GFC_Vector2D position, const char *config);

#endif
