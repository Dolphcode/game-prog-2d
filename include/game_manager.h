#ifndef __GAMEMANAGER_H__
#define __GAMEMANAGER_H__

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
void game_manager_update();

/**
 * @brief quits the game
 */
void game_quit();

#endif
