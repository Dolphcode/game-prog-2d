#include <SDL.h>

#include "simple_logger.h"

#include "gfc_shape.h"
#include "gfc_config.h"

#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

#include "tiledata.h"
#include "world.h"
#include "entity.h"
#include "light.h"
#include "camera.h"

typedef struct {
	SDL_Texture *mask;
	SDL_Texture *map;
	SDL_Renderer *renderer;
}LightManager;

static LightManager light_manager = {0};

void light_manager_init() {
	GFC_Vector2D screen_res = gf2d_graphics_get_resolution();

	light_manager.renderer = gf2d_graphics_get_renderer();
	light_manager.mask = SDL_CreateTexture(light_manager.renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_TARGET,
			screen_res.x, screen_res.y);
	light_manager.map = SDL_CreateTexture(light_manager.renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_TARGET,
			screen_res.x, screen_res.y);

	SDL_SetTextureBlendMode(light_manager.mask, SDL_BLENDMODE_MUL); // the mask is multiplied by the individual lights

	atexit(light_manager_close);
}

void light_manager_close() {
	SDL_DestroyTexture(light_manager.mask);
	SDL_DestroyTexture(light_manager.map);
}

/**
 * @brief renders the lighting overlay based on the world's entity list and obscuring map
 * @param world the world we're rendering for
 */
void light_manager_render_overlay() {
	World *world = world_get_active();
	if (!world) return;

	// The light manager renders lights based off entities, which are the only things that can have lights attached
	Entity *ents = entity_system_get();
	int ent_max = entity_system_get_max();

	// Also get camera values and get the camera box
	float zoom = main_camera_get_zoomf();
	Camera *cam = camera_get_main();
	GFC_Rect cam_bounds = cam->bounds;
	// debug bounds
	GFC_Vector2D bpos = gfc_vector2d(cam->bounds.x, cam->bounds.y);
	bpos = main_camera_calc_drawpos(bpos);
	GFC_Rect r = gfc_rect(bpos.x, bpos.y, cam->bounds.w * zoom, cam->bounds.h * zoom);
	gf2d_draw_rect(r, GFC_COLOR_WHITE);
	

	// Now we must identify which obscurers in the world are in view
	BlockingEdge2D *world_edges = malloc(world->obscurer_count * sizeof(BlockingEdge2D));
	int edges_in_view = 0;
	for (int i = 0; i < world->obscurer_count; ++i) {
		BlockingEdge2D edge = world->obscurers[i];
		GFC_Edge2D edge_conv = {edge.x1, edge.y1, edge.x2, edge.y2};
		if (gfc_point_in_rect(gfc_vector2d(edge.x1, edge.y1), cam_bounds)
			       	|| gfc_point_in_rect(gfc_vector2d(edge.x2, edge.y2), cam_bounds)
				|| gfc_edge_rect_intersection(edge_conv, cam_bounds)) {
			world_edges[edges_in_view++] = edge;
		}
	}
	slog("%d edges are in view of the camera", edges_in_view);

	// Create surfaces for drawing
	GFC_Vector2D screen_res = gf2d_graphics_get_resolution();

	SDL_Vertex curr_quad[4];
	int quad_index_order[6] = {0, 1, 2, 0, 2, 3};
	memset(curr_quad, 0, sizeof(SDL_Vertex) * 4); // Set to 0, only points will ever be set since mask is black
	for (int i = 0; i < 4; ++i) {
		curr_quad[i].color.a = 255;
	}

	// Reusable components
	SDL_Rect mask_clear = {0, 0, screen_res.x, screen_res.y};

	// For each entity check if it has lights, and perform the rendering if its light box overlaps at all with the camera view space
	int count = gfc_list_get_count(world->entity_list);
	for (int i = 0; i < ent_max; ++i) {
		Entity *ent = ents + i;
		if (!ent->_inuse) continue;

		if (ent->sources) {
			for (int j = 0; j < ent->source_count; ++j) {
				LightSource s;
				s = *(ent->sources[j]);
				
				// Check if the light is in the camera in the first place
				GFC_Circle light_rad = gfc_circle(s.offset.x + ent->position.x, s.offset.y + ent->position.y, s.range);
				if (!gfc_circle_rect_overlap(light_rad, cam_bounds)) continue;


				GFC_Vector2D drawcent;
				gfc_vector2d_add(drawcent, ent->position, s.offset);
				drawcent = main_camera_calc_drawpos(drawcent);
				gf2d_draw_circle(drawcent, s.range * zoom, s.color);
				
				// Begin drawing the mask
				/*
				SDL_Rect bg = {0, 0, screen_res.x, screen_res.y};
				Uint32 w = SDL_MapRGB(shadow_mask->format, 255, 255, 255);
				SDL_FillRect(shadow_mask, &bg, w);	
				for (int v = 0; v < edges_in_view; ++v) {
					GFC_Vector2D p1 = main_camera_calc_drawpos(gfc_vector2d(world_edges[v].x1, world_edges[v].y1)),
						     p2 = main_camera_calc_drawpos(gfc_vector2d(world_edges[v].x2, world_edges[v].y2));
					
					
				}
				
				// DEbug draw the mask
				shadow_mask_tex = SDL_CreateTextureFromSurface(renderer, shadow_mask);
				SDL_RenderCopy(renderer, shadow_mask_tex, NULL, NULL);
				SDL_DestroyTexture(shadow_mask_tex);*/
				
				// Set the render target to draw the mask
				SDL_SetRenderTarget(light_manager.renderer, light_manager.mask);
				SDL_RenderClear(light_manager.renderer);
				SDL_SetRenderDrawColor(light_manager.renderer, 255, 255, 255, 255);
				SDL_RenderFillRect(light_manager.renderer, &mask_clear);

				// Now build quads and draw them based on each obscurer
				for (int v = 0; v < edges_in_view; ++v) {
					if (world_edges[v].one_way && ent->position.y > world_edges[v].y1) continue; 

					// Start with the three points used to calculate the quad
					GFC_Vector2D p1 = main_camera_calc_drawpos(gfc_vector2d(world_edges[v].x1, world_edges[v].y1)),
						     p2 = main_camera_calc_drawpos(gfc_vector2d(world_edges[v].x2, world_edges[v].y2)),
						     p_l = main_camera_calc_drawpos(gfc_vector2d(ent->position.x, ent->position.y)),
						     p1_proj, p2_proj;

					// Project to the closest edge of the screen for p1
					gfc_vector2d_sub(p1_proj, p1, p_l);
					gfc_vector2d_normalize(&p1_proj);
					gfc_vector2d_scale_by(p1_proj, p1_proj, gfc_vector2d(100000, 100000));
					gfc_vector2d_add(p1_proj, p1_proj, p1);

					
					// Project to the closest edge of the screen for p2
					gfc_vector2d_sub(p2_proj, p2, p_l);
					gfc_vector2d_normalize(&p2_proj);
					gfc_vector2d_scale_by(p2_proj, p2_proj, gfc_vector2d(100000, 100000));
					gfc_vector2d_add(p2_proj, p2_proj, p2);

					// make the points
					SDL_FPoint fp1 = {p1.x, p1.y}, 
						   fp1_proj = {p1_proj.x, p1_proj.y},
						   fp2_proj = {p2_proj.x, p2_proj.y},
						   fp2 = {p2.x, p2.y};

					// Chug em in
					curr_quad[0].position = fp1;
					curr_quad[1].position = fp1_proj;
					curr_quad[2].position = fp2_proj;
					curr_quad[3].position = fp2;

					for (int i = 0; i < 4; ++i) {
						slog("tri %d %f %f", i, curr_quad[i].position.x, curr_quad[i].position.y);
					}

					// Draw the vertex buffer
					SDL_RenderGeometry(light_manager.renderer, NULL, curr_quad, 4, quad_index_order, 6);
					slog("rendering some cool points for edge %d", v);
				}

				// Debugging
				SDL_SetRenderTarget(light_manager.renderer, NULL);
				SDL_RenderCopy(light_manager.renderer, light_manager.mask, NULL, NULL);
			}
		}
	}

	// Return the renderer to the window
	SDL_SetRenderTarget(light_manager.renderer, NULL);

	free(world_edges);
}

/**
 * @brief loads a light source from an entity json object
 * @param json the json object to load the light source from
 */
LightSource *light_source_load(SJson *json) {
	if (!json) return NULL;
	LightSource *out = malloc(sizeof(LightSource));
	if (!out) return NULL;

	GFC_Vector2D offset;
	GFC_Vector3D color;
	float radius;
	sj_object_get_vector2d(json, "offset", &offset);
	sj_object_get_vector3d(json, "color", &color);
	sj_object_get_float(json, "radius", &radius);

	out->color = gfc_color8(color.x, color.y, color.z, 255);
	out->range = radius;
	out->offset = offset;

	return out;
}


