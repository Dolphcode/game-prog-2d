#ifndef __GAMEMANAGER_H__
#define __GAMEMANAGER_H__

#include <SDL.h>

extern Uint8 running;

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
void game_manager_update(Uint8 *keystate);

/**
 * @brief quits the game
 */
void game_quit();

// Callback functions
void load_level_1();
void load_level_2();
void load_level_3();
void open_bind_menu();
void back_to_pause();
void game_manager_bindkey(const char*);

#endif
