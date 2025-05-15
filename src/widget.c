#include "simple_logger.h"

#include "gfc_config.h"

#include "ui/widget.h"
#include "ui/label.h"
#include "ui/button.h"
#include "ui/strbutton.h"
#include "ui/minimap.h"
#include "ui/bar.h"

/*
typedef struct Widget_S {
	// Debug Stuff
	const char 		*name;		// <The name of the widget for debug purposes
	
	// Visual and Spatial Information
	Sprite			*sprite;	// <The base sprite of the widget
	GFC_Vector2D		position;	// <The screen space position of the widget
	GFC_Rect		box;		// <The box of the widget for mouse detection

	// Drawing
	Uint8			do_draw;	// <Whether this should be drawn or not
	void			(*draw)(struct Widget_S *self);

	// State
	Uint8			hovering;	// <Tracks whether the mouse is hovering over this widget or not
	Uint8			clicked;	// <Tracks whether this object is being clicked on or not

	// Event Handlers
	void			(*on_click)(struct Widget_S *self);	// <Triggered on the first frame this object is clicked on
	void			(*on_release)(struct Widget_S *self);	// <Triggered on the first frame that the mouse is no longer clicking
	void			(*on_hover_enter)(struct Widget_S *self);	// <Triggered on the first frame that the mouse starts hovering over this
	void			(*on_hover_exit)(struct Widget_S *self);	// <Triggered on the first frame that the mouse is no longer hovering over this

	// Data
	void			*data;		// <The data component of the widget
	void			(*data_free)(struct Widget_S *self);
}Widget;*/

Widget *widget_new() {
	Widget *widget = calloc(1, sizeof(Widget));
	if (!widget) {
		slog("failed to allocate memory");
		return NULL;
	}
	return widget;
}

void widget_free(Widget *self) {
	if (!self) return;

	// Free the sprite if it has one
	if (self->sprite) {
		gf2d_sprite_free(self->sprite);
	}

	// Free the data
	if (self->data && self->data_free) {
		self->data_free(self);
	}

	// Free the widget
	free(self);
}

void widget_configure_from_file(Widget *self, const char *filename) {
	if (!self || !filename) return;
	SJson *json = sj_load(filename);
	if (!json) return;
	widget_configure(self, json);
	sj_free(json);
}

void widget_configure(Widget *self, SJson *json) {
	const char *sprite = NULL;
	if (!self || !json) return;

	// Load the default sprite
	sprite = sj_object_get_string(json, "sprite");
	if (sprite) {
		GFC_Vector2D frame_size;
		Uint32 fpl;
		sj_object_get_vector2d(json, "spriteSize", &frame_size);
		sj_object_get_uint32(json, "spriteFPL", &fpl);
		self->sprite = gf2d_sprite_load_all(
			sprite,
			(Uint32)frame_size.x,
			(Uint32)frame_size.y,
			fpl,
			0
			);

	}

	// Copy the name
	const char *name = sj_object_get_string(json, "name");
	strcpy(self->name, name);	

	// Set the position and bounds of the widget in the screen
	GFC_Vector2D pos, bounds;
	sj_object_get_vector2d(json, "position", &pos);
	sj_object_get_vector2d(json, "rect", &bounds);
	self->position = pos;
	self->box.x = pos.x;
	self->box.y = pos.y;
	self->box.w = bounds.x;
	self->box.h = bounds.y;

	// Copy the color data
	GFC_Vector3D color;
	sj_object_get_vector3d(json, "color", &color);
	self->color.r = color.x;
	self->color.g = color.y;
	self->color.b = color.z;

	// Check what type of widget this is and call the appropriate configure function
	int widget_type;
	sj_object_get_int(json, "type", &widget_type);
	switch((WidgetType)widget_type) {
		case 0:
			self->draw = widget_draw_default;
			self->do_draw = 1;
			break;
		case 1:
			w_label_configure(self, json);
			break;
		case 2:
			//self->draw = widget_draw_default;
			w_button_configure(self, json);
			break;
		case 3:
			w_bar_configure(self, json);
			break;
		case 4:
			w_minimap_configure(self, json);
			self->do_draw = 1;
			break;
		case 5:
			w_strbutton_configure(self, json);
			break;
	}
	
	
	// Set default draw function for now
	//self->draw = widget_draw_default;


}

/**
 * @brief a default draw function for all widgets which simply draws its sprite
 * @param self the widget to be drawn
 */
void widget_draw_default(Widget *self) {
	if (!self || !self->sprite) return;

	gf2d_sprite_draw(
			self->sprite,
			self->position,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(Uint32)self->frame);
}

