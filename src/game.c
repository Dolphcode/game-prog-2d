#include <SDL.h>
#include <time.h>
#include <unistd.h>
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

#include "ui/window.h"
#include "ui/widget.h"

static char weapon[256];

int parse_args(int argc, char * argv[]) {
	
	int opt;
	while ((opt = getopt(argc, argv, ":cbw:")) != -1) {
		switch(opt) {
			case 'c':
				DRAW_CENTER = 1;
				break;
			case 'b':
				DRAW_BOUNDS = 1;
				break;
			case 'h':
				slog("The following command line options are valid options for this executable:\n\t-h\t\tShow help menu\n\t-c\t\tDraw entity center points\n\t-b\t\tDraw entity bounds\n\t-w path\t\tSelect weapon");
				return 1;
			case 'w':
				strcpy(weapon, optarg);
				break;
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
	/*
	for (int i = 1; i < argc; i++) {
		if (gfc_string_l_strcmp(gfc_string(argv[i]), "-h") == 0) {
			slog("The following command line options are valid options for this executable\n\t-h, --help\t\tShow help menu\n\t-c, --draw-center\t\tDraw entity center points\n\t-b, --draw-bounds\t\tDraw entity bounds\n");
			return 1;
		} else if (gfc_string_l_strcmp(gfc_string(argv[i]), "-c") == 0 || gfc_string_l_strcmp(gfc_string(argv[i]), "--draw-center") == 0) {
			DRAW_CENTER = 1;
		} else if (gfc_string_l_strcmp(gfc_string(argv[i]), "-b") == 0 || gfc_string_l_strcmp(gfc_string(argv[i]), "--draw-bounds") == 0) {
			DRAW_BOUNDS = 1;
		} else if ((gfc_string_l_strcmp(gfc_string(argv[i]), "-w") == 0 || gfc_string_l_strcmp(gfc_string(argv[i]), "--weapon") == 0) && i + 1 < argc) {
			++i;
			strcpy(weapon, argv[i]);
		}
	}*/

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
    ui_system_initialize();
    ui_system_create_window(1);
    ui_system_create_window(3);
    ui_system_create_window(5);
    ui_system_create_window(1);
    Window *win = ui_system_create_window(9);
    win->_active = 1;
    Widget *widget = widget_new();
    widget_configure_from_file(widget, "def/ui/test_sprite.def");
    window_add_widget(win, widget);
    widget->do_draw = 1;
    

    // Making a simple world and player
    World* world = world_load("def/world.def");
    world_make_active(world);

    Entity* player = spawn_entity("player", gfc_vector2d(400, 200), weapon);
    world->player = player;
    player_hud_init();

    Camera* cam = camera_get_main();
    cam->zoom = 1.0;
    cam->target = player;
   
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
	
	// Update camera

        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first
            //gf2d_sprite_draw_image(sprite,gfc_vector2d(0,0));
	    world_draw(world);

	    world_update(world_get_active());
	    // Then draw entities
	    entity_system_think_all();

	    entity_system_presync_all();
	    space_update(world_get_active()->space);
	    entity_system_postsync_all();

	    entity_system_update_all();
	    // Update camera before drawing
	    camera_update(cam);
	    entity_system_draw_all();

            //UI elements last
	    player_hud_draw();
	    ui_system_update_all();
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

    world_free(world);

    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
