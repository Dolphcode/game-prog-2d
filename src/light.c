#include "simple_logger.h"

#include "gfc_shape.h"

#include "tiledata.h"
#include "world.h"
#include "light.h"

/**
 * @brief renders the lighting overlay based on the world's entity list and obscuring map
 * @param world the world we're rendering for
 */
void light_manager_render_overlay(World *world);

/**
 * @brief loads a light source from an entity json object
 * @param json the json object to load the light source from
 */
void light_source_load(SJson *json); 


