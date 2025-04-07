#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "gfc_list.h"

#include "ui/widget.h"

typedef struct Window_S {
	Uint8		_active;	// Whether this window is active or not
	const char	name[256];	// Name for identifying windows
	int		draw_layer;	// The draw layer of this window
	GFC_List	*widgets; 	// The list of widgets attached to this window
}Window;

void ui_system_initialize();

void ui_system_close();

void ui_system_draw_all();

Window *ui_system_create_window(int draw_layer);

Window *ui_system_get_window(const char *name);

Window *window_new();

void window_free(Window *self);

void window_add_widget(Window *self, Widget *widget);

void window_sort_widgets(Window *self);

#endif
