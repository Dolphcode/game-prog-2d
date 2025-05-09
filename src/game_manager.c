#include "simple_logger.h"
#include "simple_json.h"

#include "entity.h"
#include "camera.h"
#include "world.h"
#include "game_manager.h"
#include "spawn.h"
#include "player.h"

#include "ui/window.h"
#include "ui/widget.h"
#include "ui/minimap.h"

static Uint8 level_loaded = 0;
static Uint8 paused = 0;

static Window *player_hud, *main_menu;
static Widget *minimap;

/**
 * @brief call this to initialize the game manager
 */
void game_manager_init() {
	player_hud = ui_system_get_window("player_hud");
	minimap = window_get_widget(player_hud, "minimap");
	main_menu = ui_system_get_window("main_menu_ui");
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

	World *world = world_get_active();
	world_free(world);

	level_loaded = 0;
}

/**
 * @brief game manager loop
 */
void game_manager_update() {
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

/**
 * @brief quits the game
 */
void game_quit();

void load_level_1() {
	game_manager_start_level("def/world.def");
}
void load_level_2() {
	game_manager_start_level("def/world.def");
}
void load_level_3() {
	game_manager_start_level("def/world.def");
}

