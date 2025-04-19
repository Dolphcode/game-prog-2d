#include <dirent.h>
#include <fcntl.h>

#include "simple_logger.h"

#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gfc_list.h"
#include "gfc_config.h"
#include "gfc_input.h"

#include "gf2d_draw.h"
#include "gf2d_sprite.h"
#include "gf2d_graphics.h"

#include "tiledata.h"
#include "entity.h"
#include "camera.h"

#include "ui/window.h"
#include "ui/label.h"

#include "editor/level_editor.h"

#define DEFAULT_MAP_W	100
#define DEFAULT_MAP_H	100
#define DEFAULT_TILESET		"images/tiles.png"

#define HAZARD_MAX	128
#define WAVE_MAX	10
#define ENTITY_MAX	256

#define FRAME_SIZE 	64
#define PARALLAX_FACTOR	0.05

/*
typedef struct {
	char		id[256];
	Sprite		*icon;
}Entry;

typedef struct {
	int		index;
	GFC_Rect	box;	
}Instance;

typedef enum {
	LEDIT_TILE,
	LEDIT_ENT,
	LEDIT_HAZARD
}LevelEditorMode;

typedef struct {

	// The data to be written/loaded
	int		**tilemap;		// <the tilemap dynamically allocated 2D array
	GFC_Vector 	map_size;		// <map size vector
	Sprite		*tileset;		// <the tileset sprite
	TileData 	tiledata;		// <the data object describing the tilemap for rendering
	
	Instance	hazards[HAZARD_MAX];		// <list of hazards in the level
	int		hazard_count;
	Instance	waves[WAVE_MAX][ENTITY_MAX];	// <waves and entities per wave
	int		wave_count;
	int		spawn_counts[WAVE_MAX];

	// Editor state
	int		tile_index;
	int		hazard_index;
	int		wave_index;
	int		entity_index;
	char		file_path[256];
	
}LevelEditor;*/

// The editor instance/manager
static LevelEditor editor = {0};

// List of hazards and enemies
static Entry enemy_list[256], hazard_list[256];
static int enemy_list_count = 0, hazard_list_count = 0, background_count = 0;
static char backgrounds[100][256];

// References
Window *level_editor_ui = NULL;
Widget *selected_tile_label = NULL;

LevelEditor *level_editor_get_reference() {
	return &editor;
}

void level_editor_init(const char *file_path, int new_file) {
	// Load a list of every hazard and enemy
	DIR *curr_dir;
	struct dirent *curr_ent;
	SJson *json;
	const char *sprite_path, *enemy_name;
	char json_path[512];
	GFC_Vector2D sprite_size;
	Uint32 sprite_fpl;

	curr_dir = opendir("def/enemies");
	while ((curr_ent = readdir(curr_dir)) != NULL) {
		if (curr_ent->d_name[0] == '.') continue;
		strcpy(json_path, "def/enemies/");
		strcat(json_path, curr_ent->d_name);
		json = sj_load(json_path);
		if (!json) continue;

		sprite_path = sj_object_get_string(json, "sprite");
		sj_object_get_uint32(json, "spriteFPL", &sprite_fpl);
		sj_object_get_vector2d(json, "spriteSize", &sprite_size);

		enemy_list[enemy_list_count].icon = gf2d_sprite_load_all(
				sprite_path,
				(Uint32)sprite_size.x,
				(Uint32)sprite_size.y,
				sprite_fpl,
				0);

		enemy_name = sj_object_get_string(json, "name");
		strcpy(enemy_list[enemy_list_count].id, enemy_name);
		enemy_list_count++;

		sj_free(json);
	}
	closedir(curr_dir);

	curr_dir = opendir("def/hazards");
	while ((curr_ent = readdir(curr_dir)) != NULL) {
		if (curr_ent->d_name[0] == '.') continue;
		strcpy(json_path, "def/hazards/");
		strcat(json_path, curr_ent->d_name);
		json = sj_load(json_path);
		if (!json) continue;

		sprite_path = sj_object_get_string(json, "sprite");
		sj_object_get_uint32(json, "spriteFPL", &sprite_fpl);
		sj_object_get_vector2d(json, "spriteSize", &sprite_size);

		hazard_list[hazard_list_count].icon = gf2d_sprite_load_all(
				sprite_path,
				(Uint32)sprite_size.x,
				(Uint32)sprite_size.y,
				sprite_fpl,
				0);

		enemy_name = sj_object_get_string(json, "name");
		strcpy(hazard_list[hazard_list_count].id, enemy_name);
		hazard_list_count++;

		sj_free(json);
	}
	closedir(curr_dir);

	curr_dir = opendir("images/backgrounds");
	while ((curr_ent = readdir(curr_dir)) != NULL) {
		if (curr_ent->d_name[0] == '.') continue;
		strcpy(backgrounds[background_count], "images/backgrounds/");
		strcat(backgrounds[background_count], curr_ent->d_name);
		slog("loaded background %s", backgrounds[background_count]);
	}
	closedir(curr_dir);


	if (!new_file) {
		// Load data 
		SJson *world_json = sj_load(file_path), *obj = sj_object_get_value(world_json, "world");
		
		// Load the tilemap
		SJson *vertical = sj_object_get_value(obj, "tileMap"), *horizontal = NULL, *cell = NULL;
		sj_object_get_vector2d(obj, "worldSize", &(editor.map_size));
		editor.tilemap = calloc(editor.map_size.y, sizeof(int *));
		for (int row = 0; row < editor.map_size.y; ++row) {
			editor.tilemap[row] = calloc(editor.map_size.x, sizeof(int));
			horizontal = sj_array_get_nth(vertical, row);
			for (int col = 0; col < editor.map_size.x; ++col) {
				cell = sj_array_get_nth(horizontal, col);
				sj_get_integer_value(cell, editor.tilemap[row] + col);
			}
		}
		slog("map loaded");

		// Load the tileset
		const char *tileset_path = sj_object_get_string(obj, "tileSet");
		slog("loading tileset %s", tileset_path);
		editor.tileset = gf2d_sprite_load_all(
			tileset_path,
			FRAME_SIZE,
			FRAME_SIZE,
			1,
			0);

		// Load the tiledata
		const char *tiledata_path = sj_object_get_string(obj, "tileData");
		int tiledata_count;
		SJson *tiledata_json = sj_load(tiledata_path), *data_list = sj_object_get_value(tiledata_json, "tileData"), *curr_tdata;
		sj_object_get_int(tiledata_json, "tileCount", &tiledata_count);
		editor.tile_count = tiledata_count;
		editor.tiledata = calloc(tiledata_count, sizeof(int));
		for (int i = 0; i < tiledata_count; ++i) {
			curr_tdata = sj_array_get_nth(data_list, i);
			sj_object_get_int(curr_tdata, "frame", &(editor.tiledata[i]));
		}
		sj_free(tiledata_json);

		sj_free(obj);
	} else {
		// Make the tilemap
		editor.tilemap = calloc(DEFAULT_MAP_H, sizeof(int *));
		for (int row = 0; row < DEFAULT_MAP_H; ++row) {
			editor.tilemap[row] = calloc(DEFAULT_MAP_W, sizeof(int));
		}
		editor.map_size.x = DEFAULT_MAP_W;
		editor.map_size.y = DEFAULT_MAP_H;

		// Load the default tileset
		editor.tileset = gf2d_sprite_load_all(
			DEFAULT_TILESET,
			FRAME_SIZE,
			FRAME_SIZE,
			1,
			0);

	}

	// Copy the file path into the level editor
	strcpy(editor.file_path, file_path);

	// Get UI element references
	level_editor_ui = ui_system_get_window("level_editor_ui");
	slog("got the level editor ui %p", level_editor_ui);
	selected_tile_label = window_get_widget(level_editor_ui, "tile_label");
	slog("now got the tile label");
	w_label_set_text(selected_tile_label, "Testing this thing");

	atexit(level_editor_close);
}

void level_editor_update() {
	// Get the mouse state
	int x, y, mouse_press = SDL_GetMouseState(&x, &y);

	// Compute the world position and clamp it
	GFC_Vector2D screen_pos = {x, y};
	GFC_Vector2D world_pos = main_camera_screenpos_to_worldpos(screen_pos);
	if (world_pos.x < 0) world_pos.x = 0;
	if (world_pos.x > editor.map_size.x * FRAME_SIZE) world_pos.x = (editor.map_size.x - 1) * FRAME_SIZE;
	if (world_pos.y < 0) world_pos.y = 0;
	if (world_pos.y > editor.map_size.y * FRAME_SIZE) world_pos.y = (editor.map_size.y - 1) * FRAME_SIZE;
	// Cl
	slog("%f %f", world_pos.x, world_pos.y);

	if (gfc_input_command_pressed("editor_place")) {
		int row = world_pos.y / FRAME_SIZE, col = world_pos.x / FRAME_SIZE;
		editor.tilemap[row][col] = editor.tile_index;
	}
}

void level_editor_draw() {
	for (int row = 0; row < editor.map_size.y; ++row) {
		for (int col = 0; col < editor.map_size.x; ++col) {
			int data = editor.tilemap[row][col];
			if (!(data--)) continue;


			int frame = editor.tiledata[data];
			frame--;
			
			GFC_Vector2D position = {col * FRAME_SIZE, row * FRAME_SIZE};
			GFC_Vector2D zoom = main_camera_get_zoom();
			GFC_Vector2D draw_pos = main_camera_calc_drawpos(position);


			gf2d_sprite_draw(editor.tileset,
					draw_pos,
					&zoom,
					NULL,
					NULL,
					NULL,
					NULL,
					frame);
		}
	}
}

void level_editor_inc_selection() {
	editor.tile_index++;
	if (editor.tile_index > editor.tile_count) editor.tile_index = 0;
	char buffer[256];
	sprintf(buffer, "Tile #%d", editor.tile_index);
	w_label_set_text(selected_tile_label, buffer);
}

void level_editor_dec_selection();

void level_editor_set_mode(LevelEditorMode mode);

void level_editor_save();

void level_editor_close() {
	for (int i = 0; i < enemy_list_count; ++i) {
		gf2d_sprite_free(enemy_list[i].icon);
	}
	for (int i = 0; i < hazard_list_count; ++i) {
		gf2d_sprite_free(hazard_list[i].icon);
	}

	free(editor.tiledata);
}

