#ifndef __LEVEL_EDITOR_H__
#define __LEVEL_EDITOR_H__

#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gfc_list.h"

#include "tiledata.h"
#include "entity.h"

#define HAZARD_MAX	128
#define WAVE_MAX	10
#define ENTITY_MAX	256

#define FRAME_SIZE 	64
#define PARALLAX_FACTOR	0.05

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
	GFC_Vector2D 	map_size;		// <map size vector
	Sprite		*tileset;		// <the tileset sprite
	int	 	*tiledata;		// <an array of indexes corresponding a tiledata index to a tile in the tileset
	int		tile_count;		// <the number of possible tiles in the tileset
	
	Sprite		*background;		// <bg sprite
	Sprite		*foreground;		// <fg sprite

	GFC_List	*hazards;
	GFC_List	*waves[WAVE_MAX];

	// Editor state
	LevelEditorMode		mode;
	int		tile_index;
	int		hazard_index;
	int		wave_index;
	int		entity_index;
	int		background_index;
	int		foreground_index;
	char		file_path[256];
	
}LevelEditor;

LevelEditor *level_editor_get_reference();

void level_editor_init(const char *file_path, int new_file);

void level_editor_update();

void level_editor_draw();

void level_editor_save();

void level_editor_close();

// Button Callbacks
void level_editor_inc_selection();
void level_editor_dec_selection();
void level_editor_inc_wave();
void level_editor_dec_wave();
void level_editor_inc_bg();
void level_editor_dec_bg();
void level_editor_inc_fg();
void level_editor_dec_fg();
void level_editor_tile_mode();
void level_editor_hazard_mode();
void level_editor_enemy_mode();

#endif
