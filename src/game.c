#include <SDL.h>
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

int parse_args(int argc, char * argv[]) {
	if (argc < 2) return 0;

	for (int i = 1; i < argc; i++) {
		if (gfc_string_l_strcmp(gfc_string(argv[i]), "-h") == 0) {
			slog("The following command line options are valid options for this executable\n\t-h, --help\t\tShow help menu\n\t-c, --draw-center\t\tDraw entity center points\n\t-b, --draw-bounds\t\tDraw entity bounds\n");
			return 1;
		} else if (gfc_string_l_strcmp(gfc_string(argv[i]), "-c") == 0 || gfc_string_l_strcmp(gfc_string(argv[i]), "--draw-center") == 0) {
			DRAW_CENTER = 1;
		} else if (gfc_string_l_strcmp(gfc_string(argv[i]), "-b") == 0 || gfc_string_l_strcmp(gfc_string(argv[i]), "--draw-bounds") == 0) {
			DRAW_BOUNDS = 1;
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
	
    // Parse Args
    int parse_status = parse_args(argc, argv);
    if (parse_status) return 0;

    // inserting code to initialize systems
    gfc_input_init("./config/input.cfg");
    entity_system_init(1024);

    SDL_ShowCursor(SDL_DISABLE);
    
    /*demo setup*/
    sprite = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    slog("press [escape] to quit");

    // Making a simple world and player
    World* world = world_load("def/world.def");
    world_make_active(world);

    Entity* player = player_new_entity(gfc_vector2d(-40, -40));
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

	    // Then draw entities
	    entity_system_think_all();

	    entity_system_presync_all();
	    space_update(world_get_active()->space);
	    entity_system_postsync_all();

	    entity_system_update_all();
		//slog("i'm updating everyone");		
	    // Update camera before drawing
	    camera_update(cam);
	    entity_system_draw_all();

            //UI elements last
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

    //world_free(world);

    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
