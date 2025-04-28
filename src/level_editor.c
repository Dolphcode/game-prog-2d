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
#define DEFAULT_TILESET		"images/larger_tileset.png"
#define DEFAULT_TILEDATA	"def/tiledata.def"
#define DEFAULT_TILECOUNT	3

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
static int enemy_list_count = 0, hazard_list_count = 0, background_count = 0, tiledata_count = 0;
static char backgrounds[100][256];
static char tiledatapaths[100][256];

static char file_path_str[256];

// References
Window *level_editor_ui = NULL, *tile_editor_ui = NULL, *hazard_editor_ui = NULL, *wave_editor_ui = NULL;
Widget *selected_tile_label = NULL, *selected_tile_sprite = NULL, *selected_hazard_sprite = NULL, *selected_enemy_sprite = NULL, 
       *selected_wave_label = NULL, *selected_bg_label = NULL, *selected_fg_label = NULL, *selected_tiledat_label = NULL;

LevelEditor *level_editor_get_reference() {
	return &editor;
}

void init_ui_elements() {
	// Init ui for the tile editor
	if (editor.tile_index > editor.tile_count) editor.tile_index = 1;
	char buffer[256];
	sprintf(buffer, "Tile #%d", editor.tile_index);
	w_label_set_text(selected_tile_label, buffer);
	int frame = editor.tiledata[editor.tile_index - 1];
	selected_tile_sprite->frame = frame - 1;

	// Init ui for the hazard editor
	if (editor.hazard_index >= hazard_list_count) editor.hazard_index = 0;
	selected_hazard_sprite->sprite = hazard_list[editor.hazard_index].icon;
	
	// Init ui for the entity editor
	editor.entity_index++;
	selected_enemy_sprite->sprite = enemy_list[editor.entity_index].icon;	
	sprintf(buffer, "Wave #%d", editor.wave_index + 1);
	w_label_set_text(selected_wave_label, buffer);
	
	// Init ui for the level editor
	sprintf(buffer, "%s", backgrounds[editor.background_index]);
	w_label_set_text(selected_bg_label, buffer);
	sprintf(buffer, "%s", backgrounds[editor.foreground_index]);
	w_label_set_text(selected_fg_label, buffer);
	sprintf(buffer, "%s", tiledatapaths[editor.tiledata_index]);
	w_label_set_text(selected_tiledat_label, buffer);
}

void level_editor_reload_bgs() {
	if (editor.background) {
		gf2d_sprite_free(editor.background);
		editor.background = NULL;
	}

	if (editor.foreground) {
		gf2d_sprite_free(editor.foreground);
		editor.foreground = NULL;
	}
	editor.background = gf2d_sprite_load_image(backgrounds[editor.background_index]);
	editor.foreground = gf2d_sprite_load_image(backgrounds[editor.foreground_index]);
}

void level_editor_init(const char *file_path, int new_file) {
	strcpy(file_path_str, file_path);

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
		background_count++;
	}
	closedir(curr_dir);

	curr_dir = opendir("def/tiledata");
	while ((curr_ent = readdir(curr_dir)) != NULL) {
		if (curr_ent->d_name[0] == '.') continue;
		strcpy(tiledatapaths[tiledata_count], "def/tiledata/");
		strcat(tiledatapaths[tiledata_count], curr_ent->d_name);
		slog("loaded tiledata %s", tiledatapaths[tiledata_count]);
		tiledata_count++;
	}
	closedir(curr_dir);

	// Create the lists
	editor.hazards = gfc_list_new();
	for (int i = 0; i < WAVE_MAX; ++i) {
		editor.waves[i] = gfc_list_new();
	}

	// Initialize the editor in tile mode
	editor.mode = LEDIT_TILE;
	editor.tile_index = 1;

	// Now either build the new world data or load data from a file
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

		const char *tiledata_path = sj_object_get_string(obj, "tileData");
		editor.tiledata_index = 0;
		slog("searching for tiledata %s", tiledata_path);
		for (int i = 0; i < tiledata_count; ++i) {
			slog("comparing against %s", tiledatapaths[i]);
			if (strcmp(tiledata_path, tiledatapaths[i]) == 0) {
				slog("found %s", tiledatapaths[i]);
				editor.tiledata_index = i;
				break;
			}
		}
		tiledata_path = tiledatapaths[editor.tiledata_index];

		// Load the tiledata
		int tiledata_count;
		SJson *tiledata_json = sj_load(tiledata_path), *data_list = sj_object_get_value(tiledata_json, "tileData"), *curr_tdata;
		sj_object_get_int(tiledata_json, "tileCount", &tiledata_count);
		editor.tile_count = tiledata_count;
		editor.tiledata = calloc(tiledata_count, sizeof(int));
		for (int i = 0; i < tiledata_count; ++i) {
			curr_tdata = sj_array_get_nth(data_list, i);
			sj_object_get_int(curr_tdata, "frame", &(editor.tiledata[i]));
		}
		
		// Load the tileset
		const char *tileset_path = sj_object_get_string(tiledata_json, "tileSet");
		slog("loading tileset %s", tileset_path);
		editor.tileset = gf2d_sprite_load_all(
			tileset_path,
			FRAME_SIZE,
			FRAME_SIZE,
			1,
			0);

		sj_free(tiledata_json);

		// Load the background and foreground
		const char *background_path = sj_object_get_string(obj, "background");
		editor.background_index = 0;
		slog("searching for backgorund %s", background_path);
		for (int i = 0; i < background_count; ++i) {
			slog("comparing against %s", backgrounds[i]);
			if (strcmp(background_path, backgrounds[i]) == 0) {
				slog("found %s", backgrounds[i]);
				editor.background_index = i;
				break;
			}
		}
		const char *foreground_path = sj_object_get_string(obj, "foreground");
		editor.foreground_index = 0;
		slog("searching for foreground %s", background_path);
		for (int i = 0; i < background_count; ++i) {
			slog("comparing against %s", backgrounds[i]);
			if (strcmp(foreground_path, backgrounds[i]) == 0) {
				slog("found %s", backgrounds[i]);
				editor.foreground_index = i;
				break;
			}
		}
		level_editor_reload_bgs();

		// Load the hazards
		SJson *hazards_json = sj_object_get_value(obj, "hazards"), *hazard_list_json = sj_object_get_value(hazards_json, "hazard_list"), *curr_hazard;
		Instance *curr_inst;
		int world_hazard_count;
		sj_object_get_int(hazards_json, "hazard_count", &world_hazard_count);
		for (int i = 0; i < world_hazard_count; ++i) {
			curr_inst = calloc(1, sizeof(Instance));
			curr_hazard = sj_array_get_nth(hazard_list_json, i);
			if (!curr_inst || !curr_hazard) continue;

			GFC_Vector2D position = {0};
			sj_object_get_vector2d(curr_hazard, "position", &position);

			// Match the sprite to the hazard
			const char *haz_id = sj_object_get_string(curr_hazard, "id");
			for (int j = 0; j < hazard_list_count; ++j) {
				if (strcmp(haz_id, hazard_list[j].id) == 0) {
					curr_inst->index = j;
					break;
				}
			}

			// Since conveniently all hazards are a mere tile we'll stick with this for now
			curr_inst->box.x = position.x;
			curr_inst->box.y =  position.y;
			curr_inst->box.h = 64;
			curr_inst->box.w = 64;

			// Append to the gfc_list
			gfc_list_append(editor.hazards, curr_inst);
		}


		// Load the hazards
		SJson *waves_json = sj_object_get_value(obj, "waves"), *wave_list_json = sj_object_get_value(waves_json, "wave_list");
		int world_wave_count;
		sj_object_get_int(waves_json, "wave_count", &world_wave_count);
		for (int i = 0; i < world_wave_count; ++i) {
			slog("wave %d", i + 1);
			SJson *curr_wave, *spawn_obj;
			curr_wave = sj_array_get_nth(wave_list_json, i);
			int spawn_count;
			sj_object_get_int(curr_wave, "spawn_count", &spawn_count);
			SJson *spawn_list = sj_object_get_value(curr_wave, "spawns");
			for (int j = 0; j < spawn_count; ++j) {
				SJson *curr_spawn = sj_array_get_nth(spawn_list, j);
				curr_inst = calloc(1, sizeof(Instance));
				slog("spawn %d is %p", j, curr_spawn);
				if (!curr_spawn || !curr_inst) continue;

				const char *ent_id = sj_object_get_string(curr_spawn, "id");
				for (int j = 0; j < enemy_list_count; ++j) {
					if (strcmp(ent_id, enemy_list[j].id) == 0) {
						curr_inst->index = j;
						break;
					}
				}

				Sprite *ent_sprite = enemy_list[j].icon;
		
				GFC_Vector2D position = {0};
				sj_object_get_vector2d(curr_spawn, "position", &position);

				// Since conveniently all hazards are a mere tile we'll stick with this for now
				curr_inst->box.x = position.x;
				curr_inst->box.y =  position.y;
				curr_inst->box.h = ent_sprite->frame_w;
				curr_inst->box.w = ent_sprite->frame_h;
				
				slog("appending %s to list", ent_id);
				// Append to the gfc_list
				gfc_list_append(editor.waves[i], curr_inst);

			}
		}

		sj_free(obj);
	} else {
		// Make the tilemap
		editor.tilemap = calloc(DEFAULT_MAP_H, sizeof(int *));
		for (int row = 0; row < DEFAULT_MAP_H; ++row) {
			editor.tilemap[row] = calloc(DEFAULT_MAP_W, sizeof(int));
		}
		editor.map_size.x = DEFAULT_MAP_W;
		editor.map_size.y = DEFAULT_MAP_H;

		const char *tiledata_path = tiledatapaths[0];
		int tiledata_count;
		SJson *tiledata_json = sj_load(tiledata_path), *data_list = sj_object_get_value(tiledata_json, "tileData"), *curr_tdata;
		sj_object_get_int(tiledata_json, "tileCount", &tiledata_count);
		editor.tile_count = tiledata_count;
		editor.tiledata = calloc(tiledata_count, sizeof(int));
		for (int i = 0; i < tiledata_count; ++i) {
			curr_tdata = sj_array_get_nth(data_list, i);
			sj_object_get_int(curr_tdata, "frame", &(editor.tiledata[i]));
		}

		// Load the tileset
		const char *tileset_path = sj_object_get_string(tiledata_json, "tileSet");
		slog("loading tileset %s", tileset_path);
		editor.tileset = gf2d_sprite_load_all(
			tileset_path,
			FRAME_SIZE,
			FRAME_SIZE,
			1,
			0);

		sj_free(tiledata_json);

		// Load the default background and foreground
		editor.background_index = 0;
		editor.foreground_index = 1;
		level_editor_reload_bgs();

	}

	// Copy the file path into the level editor
	strcpy(editor.file_path, file_path);

	// Get UI element references
	level_editor_ui = ui_system_get_window("level_editor_ui");
	tile_editor_ui = ui_system_get_window("tile_editor_ui");
	hazard_editor_ui = ui_system_get_window("hazard_editor_ui");
	wave_editor_ui = ui_system_get_window("wave_editor_ui");
	selected_hazard_sprite  = window_get_widget(hazard_editor_ui, "hazard_sprite");
	selected_wave_label = window_get_widget(wave_editor_ui, "wave_label");
	selected_enemy_sprite = window_get_widget(wave_editor_ui, "enemy_sprite");

	selected_tile_label = window_get_widget(tile_editor_ui, "tile_label");
	selected_tile_sprite = window_get_widget(tile_editor_ui, "tile_sprite");
	w_label_set_text(selected_tile_label, "Testing this thing");

	selected_bg_label = window_get_widget(level_editor_ui, "background_label");
	selected_fg_label = window_get_widget(level_editor_ui, "foreground_label");
	selected_tiledat_label = window_get_widget(level_editor_ui, "tiledata_label");

	// Now initialize all the ui elements
	init_ui_elements();

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
	//slog("%f %f", world_pos.x, world_pos.y);
	
	switch(editor.mode) {
		case LEDIT_TILE:
			if (gfc_input_command_down("editor_place")) {
				int row = world_pos.y / FRAME_SIZE, col = world_pos.x / FRAME_SIZE;
				editor.tilemap[row][col] = editor.tile_index;
			} else if (gfc_input_command_down("editor_remove")) {
				int row = world_pos.y / FRAME_SIZE, col = world_pos.x / FRAME_SIZE;
				editor.tilemap[row][col] = 0;
			}
			break;
		case LEDIT_ENT:
			GFC_List *curr_wave = editor.waves[editor.wave_index];
			if (gfc_input_command_pressed("editor_place")) {
				Instance *curr_inst = calloc(1, sizeof(Instance));
				Sprite *ent_sprite = enemy_list[editor.entity_index].icon;

				// Since conveniently all hazards are a mere tile we'll stick with this for now
				curr_inst->box.x = world_pos.x;
				curr_inst->box.y = world_pos.y;
				curr_inst->box.h = ent_sprite->frame_w;
				curr_inst->box.w = ent_sprite->frame_h;

				curr_inst->index = editor.entity_index; 

				// Append to the gfc_list
				gfc_list_append(curr_wave, curr_inst);
			} else if (gfc_input_command_pressed("editor_remove")) {
				int world_enemy_count = gfc_list_get_count(curr_wave);
				Instance *curr_inst;
				for (int i = world_enemy_count - 1; i >= 0; --i) {
					curr_inst = gfc_list_get_nth(curr_wave, i);
					if (world_pos.x > curr_inst->box.x &&
							world_pos.x < curr_inst->box.x + curr_inst->box.w &&
							world_pos.y > curr_inst->box.y &&
							world_pos.y < curr_inst->box.y + curr_inst->box.h) {
						gfc_list_delete_nth(curr_wave, i);
						free(curr_inst);
						//++i;
					}
				}	
			}
			break;
		case LEDIT_HAZARD:
			if (gfc_input_command_pressed("editor_place")) {
				GFC_Vector2D clamped_pos = gfc_vector2d(((int)world_pos.x / FRAME_SIZE) * FRAME_SIZE, ((int)world_pos.y / FRAME_SIZE) * FRAME_SIZE);
				Instance *curr_inst = calloc(1, sizeof(Instance));

				// Since conveniently all hazards are a mere tile we'll stick with this for now
				curr_inst->box.x = clamped_pos.x;
				curr_inst->box.y =  clamped_pos.y;
				curr_inst->box.h = FRAME_SIZE;
				curr_inst->box.w = FRAME_SIZE;

				curr_inst->index = editor.hazard_index; 

				// Append to the gfc_list
				gfc_list_append(editor.hazards, curr_inst);
			} else if (gfc_input_command_pressed("editor_remove")) {
				int world_hazard_count = gfc_list_get_count(editor.hazards);
				Instance *curr_inst;
				for (int i = world_hazard_count - 1; i >= 0; --i) {
					curr_inst = gfc_list_get_nth(editor.hazards, i);
					if (world_pos.x > curr_inst->box.x &&
							world_pos.x < curr_inst->box.x + curr_inst->box.w &&
							world_pos.y > curr_inst->box.y &&
							world_pos.y < curr_inst->box.y + curr_inst->box.h) {
						gfc_list_delete_nth(editor.hazards, i);
						free(curr_inst);
						//++i;
					}
				}	
			}
			break;
	}

}

void level_editor_draw() {
	// Get the zoom vector once and never again
	GFC_Vector2D zoom = main_camera_get_zoom();
	GFC_Vector2D screen_res_offset = gf2d_graphics_get_resolution();
	gfc_vector2d_scale_by(screen_res_offset, screen_res_offset, gfc_vector2d(0.5, 0.5));
	
	// Draw the background with parallax effect in
	GFC_Vector2D bg_center_pos = gfc_vector2d(editor.background->frame_w * 0.5, editor.background->frame_h * 0.5);
	GFC_Vector2D fg_center_pos = gfc_vector2d(editor.foreground->frame_w * 0.5, editor.foreground->frame_h * 0.5);	
	GFC_Vector2D bg_center = {0};
	GFC_Vector2D fg_center = {0};
	gfc_vector2d_add(bg_center, bg_center, main_camera_get_offset());
	gfc_vector2d_scale_by(bg_center, bg_center, gfc_vector2d(zoom.x * PARALLAX_FACTOR, zoom.y * PARALLAX_FACTOR));
	gfc_vector2d_add(bg_center, bg_center, screen_res_offset);
	gfc_vector2d_add(fg_center, fg_center, main_camera_get_offset());
	gfc_vector2d_scale_by(fg_center, fg_center, gfc_vector2d(zoom.x * 2 * PARALLAX_FACTOR, zoom.y * 2 * PARALLAX_FACTOR));
	gfc_vector2d_add(fg_center, fg_center, screen_res_offset);

	gf2d_sprite_draw(editor.background,
			bg_center,
			&zoom,
			&bg_center_pos,
			NULL,
			NULL,
			NULL,
			0);
	gf2d_sprite_draw(editor.foreground,
			fg_center,
			&zoom,
			&fg_center_pos,
			NULL,
			NULL,
			NULL,
			0);

	// Draw the tilemap
	for (int row = 0; row < editor.map_size.y; ++row) {
		for (int col = 0; col < editor.map_size.x; ++col) {
			int data = editor.tilemap[row][col];
			if (!(data--)) continue;


			int frame = editor.tiledata[data];
			frame--;
			
			GFC_Vector2D position = {col * FRAME_SIZE, row * FRAME_SIZE};
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

	// Draw the hazards
	int world_hazard_count = gfc_list_get_count(editor.hazards);
	Sprite *curr_draw;
	Instance *curr_inst;
	for (int i = 0; i < world_hazard_count; ++i) {
		curr_inst = gfc_list_get_nth(editor.hazards, i);
		if (!curr_inst) continue;

		curr_draw = hazard_list[curr_inst->index].icon;
		GFC_Vector2D draw_pos = gfc_vector2d(curr_inst->box.x, curr_inst->box.y);
		draw_pos = main_camera_calc_drawpos(draw_pos);
		gf2d_sprite_draw(curr_draw,
				draw_pos,
				&zoom,
				NULL,
				NULL,
				NULL,
				NULL,
				0);
	}

	if (editor.mode == LEDIT_ENT) {
		GFC_List *disp_wave = editor.waves[editor.wave_index];
		int spawn_count = gfc_list_get_count(disp_wave);
		slog("count %d", spawn_count);
		for (int i = 0; i < spawn_count; ++i) {
			curr_inst = gfc_list_get_nth(disp_wave, i);
			if (!curr_inst) continue;
			
			slog("printing %d at %f %f", curr_inst->index, curr_inst->box.x, curr_inst->box.y);
			curr_draw = enemy_list[curr_inst->index].icon;
			GFC_Vector2D draw_pos = gfc_vector2d(curr_inst->box.x, curr_inst->box.y);
			draw_pos = main_camera_calc_drawpos(draw_pos);
			gf2d_sprite_draw(curr_draw,
				draw_pos,
				&zoom,
				NULL,
				NULL,
				NULL,
				NULL,
				0);
		}
	}

	if (editor.mode == LEDIT_TILE || editor.mode == LEDIT_HAZARD) {
		// Draw the cursor position
		int x, y, mouse_press = SDL_GetMouseState(&x, &y);	
		GFC_Rect r = {0};
		r.w = 64 * zoom.x;
		r.h = 64 * zoom.y;
		GFC_Vector2D cursor_pos = gfc_vector2d(x, y);
		cursor_pos = main_camera_screenpos_to_worldpos(cursor_pos);
		cursor_pos.x = (int)(cursor_pos.x / 64);
		cursor_pos.y = (int)(cursor_pos.y / 64);
		cursor_pos.x = ((cursor_pos.x < 0) ? 0 : ((cursor_pos.x >= editor.map_size.x) ? editor.map_size.x - 1 : cursor_pos.x)) * 64;
		cursor_pos.y = ((cursor_pos.y < 0) ? 0 : ((cursor_pos.y >= editor.map_size.y) ? editor.map_size.y - 1 : cursor_pos.y)) * 64;
		cursor_pos = main_camera_calc_drawpos(cursor_pos);
		r.x = cursor_pos.x;
		r.y = cursor_pos.y;
		gf2d_draw_rect(r, GFC_COLOR_YELLOW);
	}
}



void level_editor_save() {
	SJson *data = sj_object_new(), *world_obj = sj_object_new();

	// Loading the basic stuff
	SJson *background = sj_new_str(backgrounds[editor.background_index]);
	SJson *foreground = sj_new_str(backgrounds[editor.foreground_index]);
	sj_object_insert(world_obj, "background", background);
	sj_object_insert(world_obj, "foreground", foreground);

	SJson *parallax_factor = sj_new_float(PARALLAX_FACTOR);
	SJson *tile_count = sj_new_int(3); // temporary
	SJson *tile_data = sj_new_str(tiledatapaths[editor.tiledata_index]);
	SJson *world_size = sj_vector2d_new(editor.map_size);
	sj_object_insert(world_obj, "parallaxFactor", parallax_factor);
	sj_object_insert(world_obj, "tileCount", tile_count);
	sj_object_insert(world_obj, "tileData", tile_data);
	sj_object_insert(world_obj, "worldSize", world_size);

	// Copy the tilemap over
	SJson *tilemap_obj = sj_array_new();
	for (int row = 0; row < editor.map_size.y; ++row) {
		SJson *row_obj = sj_array_new();
		for (int col = 0; col < editor.map_size.x; ++col) {
			SJson *cell = sj_new_int(editor.tilemap[row][col]);
			sj_array_append(row_obj, cell);
		}
		sj_array_append(tilemap_obj, row_obj);
	}
	sj_object_insert(world_obj, "tileMap", tilemap_obj);

	// Copy the hazards over
	SJson *hazards_obj = sj_object_new();
	SJson *hazard_count_obj = sj_new_int(gfc_list_get_count(editor.hazards));
	SJson *hazard_list_obj = sj_array_new();
	for (int i = 0; i < gfc_list_get_count(editor.hazards); ++i) {
		Instance *curr_inst = gfc_list_get_nth(editor.hazards, i);
		GFC_Vector2D position = gfc_vector2d(curr_inst->box.x, curr_inst->box.y);

		SJson *curr_hazard = sj_object_new();
		SJson *id = sj_new_str(hazard_list[curr_inst->index].id);
		SJson *position_obj = sj_vector2d_new(position);

		sj_object_insert(curr_hazard, "id", id);
		sj_object_insert(curr_hazard, "position", position_obj);
		sj_array_append(hazard_list_obj, curr_hazard);
	}
	sj_object_insert(hazards_obj, "hazard_count", hazard_count_obj);
	sj_object_insert(hazards_obj, "hazard_list", hazard_list_obj);
	sj_object_insert(world_obj, "hazards", hazards_obj);

	// Copy the waves over
	SJson *waves_obj = sj_object_new();
	int final_wave_count = 0;
	SJson *wave_list_obj = sj_array_new();
	for (int i = 0; i < WAVE_MAX; ++i) {
		GFC_List *curr_wave = editor.waves[i];
		if (gfc_list_get_count(curr_wave) == 0) continue;
		
		SJson *curr_wave_obj = sj_object_new();
		SJson *spawns_obj = sj_array_new();
		int final_spawn_count = 0;

		for (int j = 0; j < gfc_list_get_count(curr_wave); ++j) {
			Instance *curr_inst = gfc_list_get_nth(curr_wave, j);
			GFC_Vector2D position = gfc_vector2d(curr_inst->box.x, curr_inst->box.y);

			SJson *curr_spawn = sj_object_new();
			SJson *id = sj_new_str(enemy_list[curr_inst->index].id);
			SJson *position_obj = sj_vector2d_new(position);

			sj_object_insert(curr_spawn, "id", id);
			sj_object_insert(curr_spawn, "position", position_obj);
			sj_array_append(spawns_obj, curr_spawn);
			final_spawn_count++;
		}

		SJson *spawn_count_obj = sj_new_int(final_spawn_count);
		sj_object_insert(curr_wave_obj, "spawns", spawns_obj);
		sj_object_insert(curr_wave_obj, "spawn_count", spawn_count_obj);

		sj_array_append(wave_list_obj, curr_wave_obj);
		final_wave_count++;
	}

	SJson *wave_count_obj = sj_new_int(final_wave_count);
	sj_object_insert(waves_obj, "wave_count", wave_count_obj);
	sj_object_insert(waves_obj, "wave_list", wave_list_obj);
	sj_object_insert(world_obj, "waves", waves_obj);
	
	// Now save the  world
	sj_object_insert(data, "world", world_obj);
	sj_save(data, file_path_str);
	sj_free(data);

}

void level_editor_close() {
	// Free the enemy and hazard sprites
	for (int i = 0; i < enemy_list_count; ++i) {
		gf2d_sprite_free(enemy_list[i].icon);
	}
	for (int i = 0; i < hazard_list_count; ++i) {
		gf2d_sprite_free(hazard_list[i].icon);
	}

	// Free the foreground and background sprites if possible
	if (editor.foreground) gf2d_sprite_free(editor.foreground);
	if (editor.background) gf2d_sprite_free(editor.background);

	// Free the hazard list
	if (editor.hazards) {
		Instance *inst;
		int count = gfc_list_get_count(editor.hazards);
		for (int i = count - 1; i >= count; --i) {
			inst = gfc_list_get_nth(editor.hazards, i);
			free(inst);
			gfc_list_delete_nth(editor.hazards, i);
		}
		gfc_list_delete(editor.hazards);
	}

	// Free the wave lists
	for (int i = 0; i < WAVE_MAX; ++i) {
		if (editor.waves[i]) {
			Instance *inst;
			int count = gfc_list_get_count(editor.waves[i]);
			for (int i = count - 1; i >= count; --i) {
				inst = gfc_list_get_nth(editor.waves[i], i);
				free(inst);
				gfc_list_delete_nth(editor.waves[i], i);
			}
			gfc_list_delete(editor.waves[i]);
		}
	}

	free(editor.tiledata);
}

// Callback functions
void level_editor_inc_selection() {
	if (editor.mode == LEDIT_TILE) {
		editor.tile_index++;
		if (editor.tile_index > editor.tile_count) editor.tile_index = 1;
		char buffer[256];
		sprintf(buffer, "Tile #%d", editor.tile_index);
		w_label_set_text(selected_tile_label, buffer);
		int frame = editor.tiledata[editor.tile_index - 1];
		frame--;

		selected_tile_sprite->frame = frame;
	} else if (editor.mode == LEDIT_HAZARD) {
		editor.hazard_index++;
		if (editor.hazard_index >= hazard_list_count) editor.hazard_index = 0;
		selected_hazard_sprite->sprite = hazard_list[editor.hazard_index].icon;
	} else {
		editor.entity_index++;
		if (editor.entity_index >= enemy_list_count) editor.entity_index = 0;
		selected_enemy_sprite->sprite = enemy_list[editor.entity_index].icon;
	}

}

void level_editor_dec_selection() {
	if (editor.mode == LEDIT_TILE) {
		editor.tile_index--;
		if (editor.tile_index < 1) editor.tile_index = editor.tile_count;
		char buffer[256];
		sprintf(buffer, "Tile #%d", editor.tile_index);
		w_label_set_text(selected_tile_label, buffer);
		int frame = editor.tiledata[editor.tile_index - 1];
		frame--;

		selected_tile_sprite->frame = frame;
	} else if (editor.mode == LEDIT_HAZARD) {
		editor.hazard_index--;
		if (editor.hazard_index < 0) editor.hazard_index = hazard_list_count - 1;
		selected_hazard_sprite->sprite = hazard_list[editor.hazard_index].icon;
	} else {
		editor.entity_index--;
		if (editor.entity_index < 0) editor.entity_index = enemy_list_count - 1;
		selected_enemy_sprite->sprite = enemy_list[editor.entity_index].icon;
	}
}

void level_editor_set_mode(LevelEditorMode mode);

void level_editor_tile_mode() { 
	editor.mode = LEDIT_TILE; 
	tile_editor_ui->_active = 1;
       	hazard_editor_ui->_active = 0;
	wave_editor_ui->_active = 0;
}

void level_editor_hazard_mode() { 
	editor.mode = LEDIT_HAZARD; 	
	tile_editor_ui->_active = 0;
       	hazard_editor_ui->_active = 1;
	wave_editor_ui->_active = 0;
}

void level_editor_enemy_mode() {
	editor.mode = LEDIT_ENT; 	
	tile_editor_ui->_active = 0;
       	hazard_editor_ui->_active = 0;
	wave_editor_ui->_active = 1;
}

void level_editor_inc_wave() {
	editor.wave_index++;
	if (editor.wave_index >= WAVE_MAX) editor.wave_index = 0;
	char buffer[256];
	sprintf(buffer, "Wave #%d", editor.wave_index + 1);
	w_label_set_text(selected_wave_label, buffer);

}

void level_editor_dec_wave() {
	editor.wave_index--;
	if (editor.wave_index < 0) editor.wave_index = WAVE_MAX - 1;
	char buffer[256];
	sprintf(buffer, "Wave #%d", editor.wave_index + 1);
	w_label_set_text(selected_wave_label, buffer);
}

void level_editor_inc_bg() {
	editor.background_index++;
	if (editor.background_index >= background_count) editor.background_index = 0;
	char buffer[256];
	sprintf(buffer, "%s", backgrounds[editor.background_index]);
	w_label_set_text(selected_bg_label, buffer);
	sprintf(buffer, "%s", backgrounds[editor.foreground_index]);
	w_label_set_text(selected_fg_label, buffer);
}

void level_editor_dec_bg() {
	editor.background_index--;
	if (editor.background_index < 0) editor.background_index = background_count - 1;
	char buffer[256];
	sprintf(buffer, "%s", backgrounds[editor.background_index]);
	w_label_set_text(selected_bg_label, buffer);
	sprintf(buffer, "%s", backgrounds[editor.foreground_index]);
	w_label_set_text(selected_fg_label, buffer);
}

void level_editor_inc_fg() {
	editor.foreground_index++;
	if (editor.foreground_index >= background_count) editor.foreground_index = 0;
	char buffer[256];
	sprintf(buffer, "%s", backgrounds[editor.background_index]);
	w_label_set_text(selected_bg_label, buffer);
	sprintf(buffer, "%s", backgrounds[editor.foreground_index]);
	w_label_set_text(selected_fg_label, buffer);
}

void level_editor_dec_fg() {
	editor.foreground_index--;
	if (editor.foreground_index < 0) editor.foreground_index = background_count - 1;
	char buffer[256];
	sprintf(buffer, "%s", backgrounds[editor.background_index]);
	w_label_set_text(selected_bg_label, buffer);
	sprintf(buffer, "%s", backgrounds[editor.foreground_index]);
	w_label_set_text(selected_fg_label, buffer);
}

void level_editor_inc_tiledat() {
	editor.tiledata_index++;
	if (editor.tiledata_index >= tiledata_count) editor.tiledata_index = 0;
	char buffer[256];
	sprintf(buffer, "%s", tiledatapaths[editor.tiledata_index]);
	w_label_set_text(selected_tiledat_label, buffer);
}

void level_editor_dec_tiledat() {
	editor.tiledata_index--;
	if (editor.tiledata_index < 0) editor.tiledata_index = tiledata_count - 1;
	char buffer[256];
	sprintf(buffer, "%s", tiledatapaths[editor.tiledata_index]);
	w_label_set_text(selected_tiledat_label, buffer);
}


void level_editor_reload() {
	level_editor_reload_bgs();
		
	const char *tiledata_path = tiledatapaths[editor.tiledata_index];
	int tiledata_count;
	SJson *tiledata_json = sj_load(tiledata_path), *data_list = sj_object_get_value(tiledata_json, "tileData"), *curr_tdata;
	sj_object_get_int(tiledata_json, "tileCount", &tiledata_count);
	editor.tile_count = tiledata_count;
	editor.tiledata = calloc(tiledata_count, sizeof(int));
	for (int i = 0; i < tiledata_count; ++i) {
		curr_tdata = sj_array_get_nth(data_list, i);
		sj_object_get_int(curr_tdata, "frame", &(editor.tiledata[i]));
	}
	
	// Load the tileset (and free the original one if needed)
	if (editor.tileset) gf2d_sprite_free(editor.tileset);
	const char *tileset_path = sj_object_get_string(tiledata_json, "tileSet");
	slog("loading tileset %s", tileset_path);
	editor.tileset = gf2d_sprite_load_all(
		tileset_path,
		FRAME_SIZE,
		FRAME_SIZE,
		1,
		0);

	// Reset the selected tile if needed
	if (editor.tile_index > tiledata_count) editor.tile_index = 1;
	
	// Check the map for invalid tile indeces and zero them out
	for (int row = 0; row < editor.map_size.y; ++row) {
		for (int col = 0; col < editor.map_size.x; ++col) {
			if (editor.tilemap[row][col] > tiledata_count) editor.tilemap[row][col] = 0;
		}
	}

	init_ui_elements();	
	sj_free(tiledata_json);
}
