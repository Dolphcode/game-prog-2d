#include "simple_logger.h"
#include "simple_json.h"

#include "entity.h"
#include "camera.h"
#include "world.h"
#include "game_manager.h"
#include "player.h"

static Uint8 level_loaded = 1;
static Uint8 paused = 0;

/**
 * @brief call this to initialize the game manager
 */
void game_manager_init();

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
void game_manager_start_level(const char *);

/**
 * @brief exits the current level and returns to the main menu
 */
void game_manager_quit_level();

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
			space_update(world_get_active()->space);
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


