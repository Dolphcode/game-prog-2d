#include <unistd.h>
#include <time.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "simple_logger.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

#include "gfc_input.h"
#include "gfc_string.h"

#include "entity.h"
#include "player.h"
#include "camera.h"
#include "world.h"
#include "space.h"
#include "player.h"
#include "spawn.h"
#include "projectile.h"
#include "light.h"
#include "game_manager.h"

#include "ui/window.h"
#include "ui/widget.h"
#include "ui/minimap.h"

#include "editor/level_editor.h"

static char weapon[256];
Uint8 OPEN_LEVEL_EDITOR = 0;

static char world_file[256] = "def/world.def";
static Uint8 newfile = 0;

int parse_args(int argc, char * argv[]) {
	
	int opt;
	while ((opt = getopt(argc, argv, ":cbOun:o:l:w:")) != -1) {
		switch(opt) {
            case 'O':
                DRAW_OBSCURERS = 1;
                break;
			case 'c':
				DRAW_CENTER = 1;
				break;
			case 'b':
				DRAW_BOUNDS = 1;
				break;
			case 'u':
				DRAW_UI_BOXES = 1;
				break;
			case 'h':
				slog("The following command line options are valid options for this executable:\n\t-h\t\tShow help menu\n\t-c\t\tDraw entity center points\n\t-b\t\tDraw entity bounds\n\t-w path\t\tSelect weapon\n\t-n path\t\tMake a new level file\n\t-o path\t\tLoad an existing level and edit it");
				return 1;
			case 'w':
				strcpy(weapon, optarg);
				break;
			case 'n':
				OPEN_LEVEL_EDITOR = 1;
				newfile = 1;
				strcpy(world_file, optarg);
				break;
			case 'o':
				OPEN_LEVEL_EDITOR = 1;
				strcpy(world_file, optarg);
				break;
			case 'l':
				strcpy(world_file, optarg);
			case ':':
				switch(optopt) {
					case 'w':
						slog("Missing required 'path' option argument");
						return 1;
					break;
				}
				break;
			case '?':
				slog("Unknown option '%c'", optopt);
				break;
		}
	}	
	return 0;
}

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    Sprite *sprite;
    
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color8(255,100,255,200);
    
    /*program initializtion*/
    init_logger("gf2d.log",0);
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "gf2d",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    light_manager_init();
	
	srand((unsigned int)time(NULL));
    // Parse Args
    strcpy(weapon, "def/weapons/shotgun.def");
    int parse_status = parse_args(argc, argv);
    if (parse_status) return 0;

    // inserting code to initialize systems
    gfc_input_init("./config/input.cfg");
    entity_system_init(2048);
    projectile_pool_init();

    SDL_ShowCursor(SDL_DISABLE);
    
    /*demo setup*/
    sprite = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    slog("press [escape] to quit");

    // UI init
    TTF_Init(); // Initialize the font system and queue quitting for exit
    atexit(TTF_Quit);
    ui_system_initialize();
    if (OPEN_LEVEL_EDITOR) {
    	Window *win = ui_system_load_window("def/ui/level_editor.def");
    	win->_active = 1;
	win = ui_system_load_window("def/ui/tile_editor.def");
	win->_active = 1;
	win = ui_system_load_window("def/ui/hazard_editor.def");
	win = ui_system_load_window("def/ui/wave_editor.def");
    } else {
	Window *win = ui_system_load_window("def/ui/player_hud.def");
	//win->_active = 1;
	win = ui_system_load_window("def/ui/main_menu.def");
	win->_active = 1;
    }

    // Do all the initialization
    Entity *player;
    Camera* cam = camera_get_main();
    cam->zoom = 1.0;
    if (!OPEN_LEVEL_EDITOR) {
	game_manager_init();
	//game_manager_start_level("def/world.def");
    } else {
	player = cursor_player_spawn();
	level_editor_init(world_file, newfile);
	cam->target = player;
    }
   
    /*main game loop*/
    while(!done)
    {
        // Poll input
	gfc_input_update();

        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);
        mf+=0.1;
        if (mf >= 16.0)mf = 0;
        
        gf2d_graphics_clear_screen();// clears drawing buffers
	
	ui_system_update_all();
	
	if (OPEN_LEVEL_EDITOR) {
		// Run the level editor loop
		entity_system_think_all(); // For the camera object
		entity_system_update_all(); // For the camera object
		camera_update(cam);
		level_editor_update();
		level_editor_draw();
	} else {
		game_manager_update();
	}

	    ui_system_draw_all();
            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx,my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }

	game_manager_quit_level();

    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
