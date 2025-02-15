#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"

#include "world.h"
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

	return world;
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
	sj_object_get_uint32(world_json, "tileCount", &tile_count);
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

	// Load the tileset
	const char * tileset = sj_object_get_string(world_json, "tileSet");
	if (!tileset) {
		slog("missing 'tileSet' path");
		return NULL;
	}
	GFC_Vector2D tileset_framesize = {0};
	Uint32 tileset_fpl = 0;
	sj_object_get_vector2d(world_json, "frameSize", &tileset_framesize);
	sj_object_get_uint32(world_json, "framesPerLine", &tileset_fpl);
	world->tile_set = gf2d_sprite_load_all(
		tileset,
		(Uint32)tileset_framesize.x,
		(Uint32)tileset_framesize.y,
		tileset_fpl,
		0);
	
	// Load the tiledata
	int i;
	for (i = 0; i < tile_count; i++) {
		// Load the specific tile from the def file
		SJson *tile = sj_array_get_nth(tile_array, i);
		Uint32 frame = 0;
		int coll_type = 0;
		GFC_Vector2D coll_box = {0};
		
		// Retrieve information from def file
		sj_object_get_vector2d(tile, "collisionBox", &coll_box);
		sj_object_get_int(tile, "collisionType", &coll_type);
		sj_object_get_uint32(tile, "frame", &frame);
		
		// Load tile data into slot
		world->tile_data[i].frame = frame;
		world->tile_data[i].collision_type = (TileCollisionType)coll_type;
		world->tile_data[i].collision_box.x = coll_box.x;
		world->tile_data[i].collision_box.y = coll_box.y;
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
		}
	}

	// Create the entity list
	world->entity_list = gfc_list_new();
	if (!world->entity_list) {
		slog("failed to create entity list");
		return NULL;
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
	return;
}


