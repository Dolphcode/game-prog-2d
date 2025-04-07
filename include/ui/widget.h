#ifndef __WIDGET_H__
#define __WIDGET_H__

#include "simple_json.h"

#include "gfc_vector.h"
#include "gfc_shape.h"

#include "gf2d_sprite.h"

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
}Widget;

Widget *widget_new();

void widget_free(Widget *self);

void widget_configure_from_file(Widget *self, const char *filename);

void widget_configure(Widget *self, SJson *json);

/**
 * @brief a default draw function for all widgets which simply draws its sprite
 * @param self the widget to be drawn
 */
void widget_draw_default(Widget *self);

#endif
