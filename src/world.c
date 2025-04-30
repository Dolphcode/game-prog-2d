#include <SDL_surface.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"

#include "gf2d_draw.h"
#include "gf2d_graphics.h"

#include "projectile.h"
#include "world.h"
#include "spawn.h"

Uint8 DRAW_OBSCURERS = 0;

/*
typedef struct
{
	// Object metadata
	GFC_TextLine	name;

	// World background sprites
	Sprite		*background;	// <The static background image
	Sprite		*foreground;	// <The parallax foreground image
	float		bg_factor;	// <The parallax factor
	
	// Tileset config
	Sprite		*tile_set;	// <The tileset sprite of this world
	Uint32		tile_size;	// <The width and height of tiles in the tileset
	
	// Spatial config
	GFC_Vector2D	world_size;	// <The width and height of the world in pixels
	Tile*		tile_data;	// <An array of tile data, reserve 0 for air tiles
	Uint32*		tile_map;	// <The map of tiles in the level

	// Entities and the main camera
	Camera		*main_camera;	// <The camera object corresponding with this world
	GFC_List	*entity_list;	// <The list of entities in the list
}World;
*/


static World *active_world = NULL; // Pointer to the active world object

World *world_get_active() {
	return active_world;
}

void world_make_active(World *world) {
	if (!world) return;
	active_world = world;
}

void world_free(World *world) {
	// Verify that world pointer exists
	if (!world) return;

	// Free the background
	if (world->background) gf2d_sprite_free(world->background);
	if (world->foreground) gf2d_sprite_free(world->foreground);
	slog("freed images");
	
	// Free the tileset, tile data, and tile map
	if (world->tile_set) gf2d_sprite_free(world->tile_set);
	if (world->tile_data) free(world->tile_data);
	if (world->tile_map) free(world->tile_map);
	slog("freed tilestuff");

	// Free the entity list
	if (world->entity_list) {
		slog("freeing entity list");
		entity_system_free_list(world->entity_list);
		gfc_list_delete(world->entity_list);
	}

	// Free the space
	if (world->space) {
		slog("freeing this world's space");
		space_free(world->space);
	}

	if (world->obscurers) {
		slog("freed all obscurers");
		free(world->obscurers);
	}

	// Clear the projectile pool
	projectile_pool_clear();

	// Free the world
	free(world);
	slog("freed the world object");
}

/**
 * @brief allocates memory to create a new world object
 * @return NULL if fail, otherwise a blank world object
 */
World *world_new(Uint32 width, Uint32 height, Uint32 tile_count) {
	if (!width || !height || !tile_count) {
		slog("cannot create world with zero dimensions or zero tiles");
		return NULL;
	}
	
	// Allocate the world
	World *world;
	world = gfc_allocate_array(sizeof(World), 1);
	if (!world) {
		slog("failed to allocate memory for world");
		return NULL;
	}

	// Allocate memory for the tilemap
	world->tile_map = gfc_allocate_array(sizeof(Uint32), width * height);
	world->world_size = gfc_vector2d(width, height);

	// Allocate memory for the tiledata
	world->tile_data = gfc_allocate_array(sizeof(TileData), tile_count);
	world->tile_count = tile_count;

	// Create the space object
	world->space = space_new();
	if (!world->space) {
		slog("failed to create physics space");
		return NULL;
	}

	return world;
}

void world_build_obscurer_map(World *world) {
	if (!world) return;

	// Start by copying the tilemap to a new Uint32 array
	Uint32 *tiles = malloc(sizeof(Uint32) * world->world_size.x * world->world_size.y);
	memcpy(tiles, world->tile_map, sizeof(Uint32) * world->world_size.x * world->world_size.y);
	slog("copied the tiles");

	// Create the list of edges with the maximum possible number of edges

	// Zero out tiles that are non colliding
	for (int row = 0; row < world->world_size.y; ++row) {
		for (int col = 0; col < world->world_size.x; ++col) {
			int index = world->world_size.x * row + col;
			if (tiles[index] == 0) continue;
			TileData dat = world->tile_data[tiles[index] - 1];
			if (dat.collision_type == TCT_NONE) {
				tiles[index] = 0;
			}
		}
	}
	slog("removed noncolliding tiles");

	// Iterate through tiles to create exposed edges
	int edge_count = 0;
	BlockingEdge2D **edges = malloc(sizeof(BlockingEdge2D *) * world->world_size.x * world->world_size.y * 4);
	for (int row = 0; row < world->world_size.y; ++row) {
		for (int col = 0; col < world->world_size.x; ++col) {
			// Compute the index
			int index = world->world_size.x * row + col;

			// Skip air tiles
			if (tiles[index] == 0) continue;
			
			// Check if it's one way or not
			TileData dat = world->tile_data[tiles[index] - 1];
			int one_way = 0;
			if (dat.collision_type == TCT_ONE_WAY) {
				one_way = 1;
			}

			// Compute the world rect
			GFC_Rect world_rect = gfc_rect(col * world->tile_size, row * world->tile_size, world->tile_size, world->tile_size);

			// Check each side
			if (row == 0 || !tiles[index - (int)world->world_size.x]) { // top side
				edges[edge_count] = malloc(sizeof(BlockingEdge2D));
				BlockingEdge2D new_edge = {world_rect.x, world_rect.y, world_rect.x + world_rect.w, world_rect.y, one_way};
				*(edges[edge_count++]) = new_edge;
			}

			if ((row == world->world_size.y - 1 || !tiles[index + (int)world->world_size.x]) && dat.collision_type != TCT_ONE_WAY) { // bottom side
				edges[edge_count] = malloc(sizeof(BlockingEdge2D));
				BlockingEdge2D new_edge = {world_rect.x, world_rect.y + world_rect.h, world_rect.x + world_rect.w, world_rect.y + world_rect.h, one_way};
				*(edges[edge_count++]) = new_edge;
			}

			if ((col == 0 || !tiles[index - 1]) && dat.collision_type != TCT_ONE_WAY) { // left side
				edges[edge_count] = malloc(sizeof(BlockingEdge2D));
				BlockingEdge2D new_edge = {world_rect.x, world_rect.y, world_rect.x, world_rect.y + world_rect.h, one_way};
				*(edges[edge_count++]) = new_edge;
			}

			if ((col == world->world_size.x - 1 || !tiles[index + 1]) && dat.collision_type != TCT_ONE_WAY) { // right side
				edges[edge_count] = malloc(sizeof(BlockingEdge2D));
				BlockingEdge2D new_edge = {world_rect.x + world_rect.w, world_rect.y, world_rect.x + world_rect.w, world_rect.y + world_rect.h, one_way};
				*(edges[edge_count++]) = new_edge;
			}
		}
	}
	slog("identified exposed edges, %d", edge_count);

	// Now build the obscurers list by merging adjacent edges
	int final_edge_count = 0;
	BlockingEdge2D *obscurers = malloc(sizeof(BlockingEdge2D) * edge_count);
	for (int i = 0; i < edge_count; ++i) {
		if (edges[i] == NULL) continue;
		obscurers[final_edge_count] = *(edges[i]);
		free(edges[i]);
		edges[i] = NULL;

		int direction = (obscurers[final_edge_count].x1 == obscurers[final_edge_count].x2) ? 1 : 0; // 1 is horizontal, 0 is vertical

		for (int j = i + 1; j < edge_count; ++j) {
			if (edges[j] == NULL) continue;
			BlockingEdge2D comp = *(edges[j]);
			int comp_dir = (comp.x1 == comp.x2) ? 1 : 0;
			if (comp_dir == direction && comp.x1 == obscurers[final_edge_count].x2 && comp.y1 == obscurers[final_edge_count].y2) {
				// Free the consolidated edge
				free(edges[j]);
				edges[j] = NULL;

				// Adjust the obscurer endpoint
				obscurers[final_edge_count].x2 = comp.x2;
				obscurers[final_edge_count].y2 = comp.y2;
			}
		}
		final_edge_count++;
	}
	obscurers = realloc(obscurers, sizeof(BlockingEdge2D) * final_edge_count);
	world->obscurers = obscurers;
	world->obscurer_count = final_edge_count;
	slog("World has %d edges", final_edge_count);

	// Reallocate memory for the obscurer list for efficiency and free the tiles
	free(edges);
	free(tiles);
}

void world_build_tile_layer(World *world) {
	if (!world) return;

	// Initialize variables
	GFC_Vector2D position = {0};
	Uint32 frame;
	int i, j, index;

	// Create the blank surface
	world->tile_layer = gf2d_sprite_new();
	if (!world->tile_layer) {
		slog("failed to allocate memory for tile layer");
		return;
	}
	/*
	world->tile_layer->surface = SDL_CreateRGBSurface(
		0, // Flags not used???
		world->world_size.x * world->tile_size,
		world->world_size.y * world->tile_size,
		32,
		0, // No R mask
		0, // No G mask
		0, // No B mask
		0  // No A mask
	);*/
	world->tile_layer->surface = gf2d_graphics_create_surface(world->world_size.x * world->tile_size, world->world_size.y * world->tile_size);
	slog("printing the world size %f %f %i", world->world_size.x, world->world_size.y, world->tile_size);
	if (!world->tile_layer->surface) {
		slog("failed to create surface");
		return;
	}
	
	// Iterate over tilemap and draw tiles
	for (i = 0; i < world->world_size.y; i++) {
		for (j = 0; j < world->world_size.x; j++) {
			index = i * world->world_size.x + j;

			position.x = j * world->tile_size;
			position.y = i * world->tile_size;
			frame = world->tile_map[index] - 1;
			if (frame < 0) continue;

			gf2d_sprite_draw_to_surface(
				world->tile_set,
				position,
				NULL,
				NULL,
				frame,
				world->tile_layer->surface
			);
		}
	}

	// Render the texture
	world->tile_layer->texture = SDL_CreateTextureFromSurface(
		gf2d_graphics_get_renderer(),
		world->tile_layer->surface
	);
	if (!world->tile_layer->texture) {
		slog("failed to convert texture to surface");
		return;
	}

	// Set frame_w and frame_h for ease of access
	world->tile_layer->frame_w = world->world_size.x * world->tile_size;
	world->tile_layer->frame_h = world->world_size.y * world->tile_size;
}

/**
 * @brief loads a world object from a filename
 * @param filename the path to the def file for the world we are loading
 * @return NULL if fail, otherwise the world object being loaded
 */
World *world_load(const char *filename) {
	// Load the world json object
	if (!filename) {
		slog("no world def filename provided");
		return NULL;
	}
	SJson *json = sj_load(filename);
	if (!json) {
		slog("failed to load filename %s", filename);
		return NULL;
	}

	// Get the world object
	SJson *world_json = sj_object_get_value(json, "world");
	if (!world_json) {
		slog("missing 'world' object");
		return NULL;
	}

	// Load the tileset json file
	const char *tiledata_filename = sj_object_get_string(world_json, "tileData");
	if (!tiledata_filename) {
		slog("missing 'tileData' path");
		return NULL;
	}
	SJson *tile_json = sj_load(tiledata_filename);
	if (!tile_json) {
		slog("failed to load tiledata file %s", tiledata_filename);
		return NULL;
	}
	SJson *tile_array = sj_object_get_value(tile_json, "tileData");
	if (!tile_array) {
		slog("missing 'tileData' array");
		return NULL;
	}

	// Create the world object
	Uint32 tile_count = 0;
	GFC_Vector2D world_size = {0};
	sj_object_get_uint32(tile_json, "tileCount", &tile_count);
	slog("creating for tilecount %i", tile_count);
	sj_object_get_vector2d(world_json, "worldSize", &world_size);
	World* world = world_new(world_size.x, world_size.y, tile_count);
	if (!world) {
		slog("failed to create world object");
		return NULL;
	}

	// Load the background
	const char * background = sj_object_get_string(world_json, "background");
	if (!background) {
		slog("missing 'background' path");
		return NULL;
	}
	world->background = gf2d_sprite_load_image(background);
	
	// Load the foreground
	const char * foreground = sj_object_get_string(world_json, "foreground");
	if (!foreground) {
		slog("missing 'foreground' path");
		return NULL;
	}
	world->foreground = gf2d_sprite_load_image(foreground);

	// Get the parallax scale factor
	sj_object_get_float(world_json, "parallaxFactor", &world->bg_factor);

	// Load the tileset
	const char * tileset = sj_object_get_string(tile_json, "tileSet");
	if (!tileset) {
		slog("missing 'tileSet' path");
		return NULL;
	}
	Uint32 tileset_framesize = 0;
	Uint32 tileset_fpl = 0;
	sj_object_get_uint32(tile_json, "frameSize", &tileset_framesize);
	sj_object_get_uint32(tile_json, "framesPerLine", &tileset_fpl);
	world->tile_set = gf2d_sprite_load_all(
		tileset,
		tileset_framesize,
		tileset_framesize,
		tileset_fpl,
		1);

	world->tile_size = tileset_framesize;
	
	// Load the tiledata
	int i;
	for (i = 0; i < tile_count; i++) {
		// Load the specific tile from the def file
		SJson *tile = sj_array_get_nth(tile_array, i);
		Uint32 frame = 0;
		int coll_type = 0;
		GFC_Vector2D coll_box = {0};
		GFC_Vector3D minimap_color = {0};
		
		// Retrieve information from def file
		sj_object_get_vector2d(tile, "collisionBox", &coll_box);
		sj_object_get_int(tile, "collisionType", &coll_type);
		sj_object_get_uint32(tile, "frame", &frame);
		sj_object_get_vector3d(tile, "tileColor", &minimap_color);
		
		// Load tile data into slot
		world->tile_data[i].frame = frame;
		world->tile_data[i].collision_type = (TileCollisionType)coll_type;
		world->tile_data[i].collision_box.x = coll_box.x;
		world->tile_data[i].collision_box.y = coll_box.y;
		world->tile_data[i].minimap_color = gfc_color8(minimap_color.x, minimap_color.y, minimap_color.z, 255);
	}
	
	// Load the tilemap
	SJson *vertical = sj_object_get_value(world_json, "tileMap");
	SJson *horizontal = NULL;
	SJson *item = NULL;
	int tile_value = 0;
	if (!vertical) {
		slog("'tileMap' not provided");
		return NULL;
	}

	int row, col;
	for (row = 0; row < world_size.y; row++) {
		horizontal = sj_array_get_nth(vertical, row);
		if (!horizontal) continue;

		for (col = 0; col < world_size.x; col++) {
			item = sj_array_get_nth(horizontal, col);
			if (!item) continue;
			sj_get_integer_value(item, &tile_value);
			world->tile_map[row * (int)world_size.x + col] = tile_value;


			// Append the tile to the world's space
			if (world->space && tile_value && world->tile_data[tile_value - 1].collision_type) {
				space_add_static_rect(world->space, gfc_rect(col * world->tile_size, row * world->tile_size, world->tile_size, world->tile_size),
						world->tile_data[tile_value - 1].collision_type);
			}
		}
	}

	// Build the tile layer, then load teh obscurers
	world_build_tile_layer(world);
	world_build_obscurer_map(world);

	// Create the entity list
	world->entity_list = gfc_list_new();
	if (!world->entity_list) {
		slog("failed to create entity list");
		return NULL;
	}

	// Spawn the hazards
	SJson *hazard_json = sj_object_get_value(world_json, "hazards");
	if (!hazard_json) {
		slog("no hazard list provided, no hazards will be spawned");
	} else {
		SJson *hazard_list, *hazard_obj;
		int hazard_count;
		const char *hazard_name;
		GFC_Vector2D hazard_pos;

		sj_object_get_int(hazard_json, "hazard_count", &hazard_count);
		hazard_list = sj_object_get_value(hazard_json, "hazard_list");
		if (!hazard_list) {
			slog("invalid hazard list, no hazards will be spawned");
		} else {
			slog("reading the hazard list");
			for (hazard_count -= 1; hazard_count >= 0; hazard_count--) {
				hazard_obj = sj_array_get_nth(hazard_list, hazard_count);
				if (!hazard_obj) continue;
				hazard_name = sj_object_get_string(hazard_obj, "id");
				//slog("hazard name: %s", hazard_name);
				sj_object_get_vector2d(hazard_obj, "position", &hazard_pos);
				//slog("spawn at position %f %f", hazard_pos.x, hazard_pos.y);
				Entity *ent = spawn_entity_default(hazard_name, hazard_pos);
				gfc_list_append(world->entity_list, ent);
				space_add_entity(world->space, ent);
				slog("spawning a hazard at %f %f", hazard_pos.x, hazard_pos.y);
			}
		}
	}

	// Setup waves
	SJson *waves_json = sj_object_get_value(world_json, "waves");
	if (!waves_json) {
		slog("No waves provided");
		return NULL;
	}
	int wave_count;
	sj_object_get_int(waves_json, "wave_count", &wave_count);
	world->wave_count = wave_count; // Store the wave count
	world->curr_wave = -1;
	
	// Load the waves
	SJson *wave_list, *wave_obj, *spawn_list, *spawn_obj;
	wave_list = sj_object_get_value(waves_json, "wave_list");
	if (!wave_list) {
		slog("Wave list couldn't be loaded");
		return NULL;
	}

	for (wave_count -= 1; wave_count >= 0; --wave_count) {
		wave_obj = sj_array_get_nth(wave_list, wave_count);
		if (!wave_obj) {
			slog("Failed to load wave data, FATAL error");
			return NULL;
		}

		// Get the wave to be modified
		Wave *wave_ptr = &(world->waves[wave_count]);

		// Now load the spawn data
		int spawn_count;
		sj_object_get_int(wave_obj, "spawn_count", &spawn_count);
		wave_ptr->spawn_count = spawn_count;
		
		spawn_list = sj_object_get_value(wave_obj, "spawns");
		for (spawn_count -= 1; spawn_count >= 0; --spawn_count) {
			spawn_obj = sj_array_get_nth(spawn_list, spawn_count);

			WaveSpawn *spawn_ptr = &(wave_ptr->spawns[spawn_count]);
			const char *spawn_id = sj_object_get_string(spawn_obj, "id");
			strcpy(spawn_ptr->id, spawn_id);
			slog("loading %s", spawn_ptr->id);
			sj_object_get_vector2d(spawn_obj, "position", &(spawn_ptr->pos));
		}
	}

	
	// Free the json objects
	sj_free(json);
	sj_free(tile_json);

	return world;
}

/**
 * @brief draws the world's tilemap
 * @param world the world object to be drawn
 */
void world_draw(World *world) {
	// Get the camera object
	Camera* camera = camera_get_main();

	// Calculate scale
	GFC_Vector2D scale = main_camera_get_zoom(camera);

	// Get centers
	GFC_Vector2D bg_center = gfc_vector2d(world->background->frame_w * 0.5, world->background->frame_h * 0.5);
	GFC_Vector2D fg_center = gfc_vector2d(world->foreground->frame_w * 0.5, world->foreground->frame_h * 0.5);

	// Get half the screen res for repositioning
	GFC_Vector2D screen_res_offset = gf2d_graphics_get_resolution();
	gfc_vector2d_scale_by(screen_res_offset, screen_res_offset, gfc_vector2d(0.5, 0.5));

	// Calculate draw position of the background
	GFC_Vector2D bg_pos_scale = gfc_vector2d(scale.x * world->bg_factor, scale.y * world->bg_factor);
	GFC_Vector2D bg_draw_pos = {0};
	gfc_vector2d_add(bg_draw_pos, bg_draw_pos, main_camera_get_offset());
	gfc_vector2d_scale_by(bg_draw_pos, bg_draw_pos, bg_pos_scale);
	gfc_vector2d_add(bg_draw_pos, bg_draw_pos, screen_res_offset);
	
	// Calculate draw position of the foreground
	GFC_Vector2D fg_pos_scale = gfc_vector2d(scale.x * 2.0 * world->bg_factor, scale.y * 2.0 * world->bg_factor);
	GFC_Vector2D fg_draw_pos = {0};
	gfc_vector2d_add(fg_draw_pos, fg_draw_pos, main_camera_get_offset());
	gfc_vector2d_scale_by(fg_draw_pos, fg_draw_pos, fg_pos_scale);
	gfc_vector2d_add(fg_draw_pos, fg_draw_pos, screen_res_offset);

	// Draw the background and foreground
	gf2d_sprite_draw(world->background,
			bg_draw_pos,
			&scale,
			&bg_center,
			NULL,
			NULL,
			NULL,
			0);

	gf2d_sprite_draw(world->foreground,
			fg_draw_pos,
			&scale,
			&fg_center,
			NULL,
			NULL,
			NULL,
			0);
	
	// For drawing the tiles themselves we will not change the center
	// (0,0) is where the player spawns right now, as well as the top left bound of the map
	GFC_Vector2D tile_layer_draw_pos = {0};
	gfc_vector2d_add(tile_layer_draw_pos, tile_layer_draw_pos, main_camera_get_offset());
	gfc_vector2d_scale_by(tile_layer_draw_pos, tile_layer_draw_pos, scale);
	gfc_vector2d_add(tile_layer_draw_pos, tile_layer_draw_pos, screen_res_offset);

	gf2d_sprite_draw(world->tile_layer,
			tile_layer_draw_pos,
			&scale,
			NULL,
			NULL,
			NULL,
			NULL,
			0);

	if (DRAW_BOUNDS) space_draw(world->space);

	if (DRAW_OBSCURERS) {
		for (int n = 0; n < world->obscurer_count; ++n) {
			GFC_Vector2D p1 = gfc_vector2d(world->obscurers[n].x1, world->obscurers[n].y1);
			GFC_Vector2D p2 = gfc_vector2d(world->obscurers[n].x2, world->obscurers[n].y2);
			slog("Drawing a line %d from %f %f to %f %f", n, p1.x, p1.y, p2.x, p2.y);
			p1 = main_camera_calc_drawpos(p1);
			p2 = main_camera_calc_drawpos(p2);
			gf2d_draw_line(p1, p2, GFC_COLOR_BROWN);
		
		}
	}
}

static int wave = 0;

void world_update(World *world) {
	if (!world) return;

	int all_dead = 1;
	// Check if all entities are dead
	if (world->curr_wave >= 0) {
		Wave *wave_ptr = &(world->waves[world->curr_wave]);
		for (int i = 0; i < wave_ptr->spawn_count; ++i) {
			if (wave_ptr->ents[i]->alive) {
				all_dead = 0;
				break;
			}
		}
	}

	// Advance the wave
	if (world->curr_wave < 0 || all_dead) {
		world->curr_wave += 1;
		slog("loading wave %d", world->curr_wave);
		Wave *wave_ptr = &(world->waves[world->curr_wave]);
		WaveSpawn spawndata;
		for (int i = 0; i < wave_ptr->spawn_count; ++i) {
			// Spawn every enemy
			spawndata = wave_ptr->spawns[i];
			slog("spawning a %s %f %f", wave_ptr->spawns[i].id, wave_ptr->spawns[i].pos.x, wave_ptr->spawns[i].pos.y);
			wave_ptr->ents[i] = spawn_entity_default(spawndata.id, spawndata.pos);
		}	
	}
	/*
	Entity *curr;
	if (wave == 0) {
		wave++;
		for (int i = 0; i < 5; ++i) {
    			curr = spawn_entity("shotgunner", gfc_vector2d(400 + 100 * i, 400), "def/enemies/shotgunner.def");
			gfc_list_append(world->entity_list, curr);
		}
	} else if (wave == 1) {
		int all_dead = 1, count;
		count = gfc_list_count(world->entity_list);
		for (int i = 0; i < count; ++i) {
			curr = gfc_list_get_nth(world->entity_list, i);
			if (!curr) continue;

			if (curr->alive) all_dead = 0;
		}
		if (all_dead) {
			wave++;

			// Clear the previous set
			for (int i = count - 1; i >= 0; --i) {
				curr = gfc_list_get_nth(world->entity_list, i);
				entity_free(curr);
				gfc_list_delete_nth(world->entity_list, i);
			}

			for (int i = 0; i < 2; ++i) {
    				curr = spawn_entity("snipergunner", gfc_vector2d(1000 + 100 * i, 400), "def/enemies/snipergunner.def");	
				gfc_list_append(world->entity_list, curr);
			}			
			for (int i = 0; i < 2; ++i) {
    				curr = spawn_entity("minigunner", gfc_vector2d(400 + 100 * i, 400), "def/enemies/minigunner.def");
				gfc_list_append(world->entity_list, curr);
			}
		}
	} else if (wave == 2) {
		int all_dead = 1, count;
		count = gfc_list_count(world->entity_list);
		for (int i = 0; i < count; ++i) {
			curr = gfc_list_get_nth(world->entity_list, i);
			if (!curr) continue;

			if (curr->alive) all_dead = 0;
		}
		if (all_dead) {
			wave++;

			// Clear the previous set
			for (int i = count - 1; i >= 0; --i) {
				curr = gfc_list_get_nth(world->entity_list, i);
				entity_free(curr);
				gfc_list_delete_nth(world->entity_list, i);
			}

			for (int i = 0; i < 2; ++i) {
    				curr = spawn_entity("circlegunner", gfc_vector2d(400 + 100 * i, 400), "def/enemies/circlegunner.def");
				gfc_list_append(world->entity_list, curr);
			}			
			for (int i = 0; i < 2; ++i) {
    				curr = spawn_entity("rammer", gfc_vector2d(400 + 100 * i, 400), "def/enemies/rammer.def");
				gfc_list_append(world->entity_list, curr);
			}
		}
	} else if (wave == 3) {
		int all_dead = 1, count;
		count = gfc_list_count(world->entity_list);
		for (int i = 0; i < count; ++i) {
			curr = gfc_list_get_nth(world->entity_list, i);
			if (!curr) continue;

			if (curr->alive) all_dead = 0;
		}
		if (all_dead) {
			wave++;
			spawn_entity("electroworm", gfc_vector2d(400, 400), "def/boss/electroworm_head.def");
		}
	}*/
}
