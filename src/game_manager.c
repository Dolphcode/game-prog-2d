#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_input.h"

#include "entity.h"
#include "camera.h"
#include "world.h"
#include "game_manager.h"
#include "spawn.h"
#include "player.h"

#include "ui/window.h"
#include "ui/widget.h"
#include "ui/minimap.h"
#include "ui/label.h"

static Uint8 level_loaded = 0;
static Uint8 paused = 0;
Uint8 running = 1;

static Window *player_hud, *main_menu, *pause_menu, *bind_menu;
static Widget *minimap;
static int active_menu = 0; // 0 = pause, 1 = bind

static Uint8 binding = 0;
static char binding_ctrl[256];
static SJson *controls;

Uint8 game_manager_bind_convert(char *, SDL_Scancode);
void game_manager_update_binds();

void game_manager_close() {
	if (controls) sj_free(controls);
}

/**
 * @brief call this to initialize the game manager
 */
void game_manager_init() {
	player_hud = ui_system_get_window("player_hud");
	minimap = window_get_widget(player_hud, "minimap");
	main_menu = ui_system_get_window("main_menu_ui");

	pause_menu = ui_system_get_window("pause_menu_ui");
	bind_menu = ui_system_get_window("bind_menu_ui");

	controls = sj_load("config/input.cfg");
	if (!controls) {
		slog("WARNING: Couldn't load input config");
	}
	game_manager_update_binds();
	atexit(game_manager_close);
}

/**
 * @brief returns true if the game is paused
 */
int game_manager_get_paused();

/**
 * @brief sets the pause value of the game manager
 */
void game_manager_set_paused(int state);

/**
 * @brief start the level
 */
void game_manager_start_level(const char *world_file) {
	World *world = world_load(world_file);
	world_make_active(world);

	Entity *player = spawn_entity("player", gfc_vector2d(400, 200), "def/weapons/gatling.def");
	player_hud_init();
	Camera *cam = camera_get_main();
	cam->target = player;

	world->player = player;

	w_minimap_map_config(minimap, world, player);
	player_hud->_active = 1;
	main_menu->_active = 0;

	gfc_list_append(world->entity_list, player);

	level_loaded = 1;
	paused = 0;
}

/**
 * @brief exits the current level and returns to the main menu
 */
void game_manager_quit_level() {
	if (!level_loaded) return;
	player_hud->_active = 0;
	main_menu->_active = 1;
	paused = 0;
	active_menu = 0;

	World *world = world_get_active();
	world_free(world);

	level_loaded = 0;
}

/**
 * @brief game manager loop
 */
void game_manager_update(Uint8 *keystate) {
	if (binding) {
		char newkey[256];
		for (int i = SDL_SCANCODE_A; i < SDL_SCANCODE_NUMLOCKCLEAR; i++) {
			if (keystate[i] && game_manager_bind_convert(newkey, i)) {
				// Bind
				binding = 0;

				// Now perform the bind
				SJson *controls_obj = sj_object_get_value(controls, "commands"), *cmd, *bind, *bindval;
				int i = 0;
				while ((cmd = sj_array_get_nth(controls_obj, i)) != NULL) {
					const char *key = sj_object_get_string(cmd, "command");
					if (strcmp(key, binding_ctrl) == 0) {
						sj_object_delete_key(cmd, "keys");
						bind = sj_array_new();
						bindval = sj_new_str(newkey);
						sj_array_append(bind, bindval);
						sj_object_insert(cmd, "keys", bind);
					}
					i++;
				}

				// Perform the write and update
				sj_save(controls, "config/input.cfg");
				gfc_input_commands_purge();
				gfc_input_commands_load("./config/input.cfg");
				game_manager_update_binds();
				break;
			}
		}	
		return;
	}

	if (gfc_input_command_pressed("pause_game")) {
		paused = !paused;
	}
	
	pause_menu->_active = 0;
	bind_menu->_active = 0;
	if (paused) {
		switch(active_menu) {
			case 0:
				pause_menu->_active = 1;
				break;
			case 1:
				bind_menu->_active = 1;
				break;
		}
	}

	if (level_loaded) {
		World *world = world_get_active();
		Camera *cam = camera_get_main();
		world_draw(world);
		if (!paused) {
			world_update(world);
			entity_system_think_all();
			entity_system_presync_all();
			space_update(world->space);
			entity_system_postsync_all();
			entity_system_update_all();
		}
		camera_update(cam);
		entity_system_draw_all();
		light_manager_render_overlay();
		player_hud_draw(); // TEMPORARY	
	}
}

// KEYBINDING

void game_manager_update_binds() {
	// Now perform the bind
	SJson *controls_obj = sj_object_get_value(controls, "commands"), *cmd, *bind, *bindval;
	int i = 0;
	while ((cmd = sj_array_get_nth(controls_obj, i)) != NULL) {
		const char *key = sj_object_get_string(cmd, "command");
		const char *prefix = "key_label_";
		char buf[256];
		strcpy(buf, prefix);
		strcat(buf, key);

		Widget *w = window_get_widget(bind_menu, buf);
		if (w) {
			bind = sj_object_get_value(cmd, "keys");
			bindval = sj_array_get_nth(bind, 0);
			const char *key_text = sj_get_string_value(bindval);
			w_label_set_text(w, key_text);
		}

		i++;
	}
}


void game_manager_bindkey(const char* key) {
	strcpy(binding_ctrl, key);
	binding = 1;
}

Uint8 game_manager_bind_convert(char *buf, SDL_Scancode k) {
	if (k >= SDL_SCANCODE_A && k <= SDL_SCANCODE_Z) {
		buf[0] = 'a' + (k - SDL_SCANCODE_A);
		buf[1] = 0;
		return 1;
	} else if (k == SDL_SCANCODE_0) {
		buf[0] = '0';
		buf[1] = 0;
		return 1;
	} else if (k >= SDL_SCANCODE_1 && k <= SDL_SCANCODE_9) {
		buf[0] = '1' + (k - SDL_SCANCODE_1);
		buf[1] = 0;
		return 1;
	} else if (k >= SDL_SCANCODE_F1 && k <= SDL_SCANCODE_F12) {
		int n = 1 + k - SDL_SCANCODE_F1;
		buf[0] = 'F';
		sprintf(buf + 1, "%d", n);
		return 1;
	} else {
		char *tmp = NULL;
		switch(k) {
			case SDL_SCANCODE_MINUS:
				tmp = "-";
				break;
			case SDL_SCANCODE_EQUALS:
				tmp = "=";
				break;
			case SDL_SCANCODE_LEFTBRACKET:
				tmp = "[";
				break;
			case SDL_SCANCODE_RIGHTBRACKET:
				tmp = "]";
				break;
			case SDL_SCANCODE_PERIOD:
				tmp = ".";
				break;
			case SDL_SCANCODE_COMMA:
				tmp = ",";
				break;
			case SDL_SCANCODE_SEMICOLON:
				tmp = ";";
				break;
			case SDL_SCANCODE_BACKSLASH:
				tmp = "\\";
				break;
			case SDL_SCANCODE_SLASH:
				tmp = "/";
				break;
			case SDL_SCANCODE_APOSTROPHE:
				tmp = "\'";
				break;
			case SDL_SCANCODE_GRAVE:
				tmp = "`";
				break;
			case SDL_SCANCODE_SPACE:
				tmp = " ";
				break;
			case SDL_SCANCODE_BACKSPACE:
				tmp = "BACKSPACE";
				break;
			case SDL_SCANCODE_RIGHT:
				tmp = "RIGHT";
				break;
			case SDL_SCANCODE_LEFT:
				tmp = "LEFT";
				break;
			case SDL_SCANCODE_UP:
				tmp = "UP";
				break;
			case SDL_SCANCODE_DOWN:
				tmp = "DOWN";
				break;
			case SDL_SCANCODE_LALT:
				tmp = "LALT";
				break;
			case SDL_SCANCODE_RALT:
				tmp = "RALT";
				break;
			case SDL_SCANCODE_LSHIFT:
				tmp = "LSHIFT";
				break;
			case SDL_SCANCODE_RSHIFT:
				tmp = "RSHIFT";
				break;
			case SDL_SCANCODE_TAB:
				tmp = "TAB";
				break;
			case SDL_SCANCODE_RETURN:
				tmp = "RETURN";
				break;
			case SDL_SCANCODE_DELETE:
				tmp = "DELETE";
				break;
			case SDL_SCANCODE_ESCAPE:
				tmp = "ESCAPE";
				break;
		}
		if (tmp) {
			strcpy(buf, tmp);
			return 1;
		}
	}
	return 0;
}

// CALLBACKS
/**
 * @brief quits the game
 */
void reset_binds() {
	FILE *default_config, *config;
	default_config = fopen("config/default_input.cfg", "r");
	config = fopen("config/input.cfg", "w");
	char c;
	while ((c = fgetc(default_config)) != EOF) {
		fputc(c, config);
	}
	fclose(config);
	fclose(default_config);
	sj_free(controls);
	controls = sj_load("config/input.cfg");
	gfc_input_commands_purge();
	gfc_input_commands_load("./config/input.cfg");
	game_manager_update_binds();

}
void game_quit() {
	running = 0;
}
void load_level_param(const char* level) {
	game_manager_start_level(level);
}
void load_level_1() {
	//game_manager_start_level("def/bosstest.def");
	game_manager_start_level("def/world.def");
}
void load_level_2() {
	game_manager_start_level("def/world.def");
}
void load_level_3() {
	game_manager_start_level("def/world.def");
}
void open_bind_menu() {
	active_menu = 1;
}
void back_to_pause() {
	active_menu = 0;
}

