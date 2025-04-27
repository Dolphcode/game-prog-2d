#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "simple_json.h"

#include "gfc_vector.h"
#include "gfc_color.h"


typedef struct LightSource_S {
	GFC_Vector2D	offset;	// <The offset from the center of the parent where this light source should be drawn
	float		range;	// <The range of the light
	GFC_Color	color;	// <The color of the light
}LightSource;

/**
 * @brief renders the lighting overlay based on the world's entity list and obscuring map
 * @param world the world we're rendering for
 */
void light_manager_render_overlay();

/**
 * @brief loads a light source from an entity json object
 * @param json the json object to load the light source from
 */
void light_source_load(SJson *json); 



#endif
