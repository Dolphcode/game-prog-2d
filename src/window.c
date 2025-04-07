#include "simple_logger.h"
#include "simple_json.h"

#include "ui/window.h"

/*
typedef struct Window_S {
	Uint8		_active;	// Whether this window is active or not
	GFC_List	*widgets; 	// The list of widgets attached to this window
}Window;
*/

static GFC_List *ui_system = NULL;

void ui_system_initialize() {
	// Create the ui system variable
	ui_system = gfc_list_new();
	if (!ui_system) {
		slog("failed to initialize ui system");
		return;
	}

	// Queue closing for exit at this point
	atexit(ui_system_close);
	slog("ui system initialized");
}

void ui_system_close() {
	// Free each window in the ui system
	if (!ui_system) return;
	int count = gfc_list_count(ui_system), i;
	for (i = count - 1; i >= 0; --i) {
		// Free the window object individually
		window_free(gfc_list_get_nth(ui_system, i));

		// Free the slot
		gfc_list_delete_nth(ui_system, i);
	}

	// Free the list
	gfc_list_delete(ui_system);
	slog("ui system freed");
}

void ui_system_draw_all() {
	int count, i, widget_count, j;
	Window *curr;
	Widget *curr_widget;

	count = gfc_list_count(ui_system);
	for (i = 0; i < count; ++i) {
		curr = gfc_list_get_nth(ui_system, i);
		slog("drawing window with draw_layer %d", curr->draw_layer);
		widget_count = gfc_list_count(curr->widgets);
		for (j = 0; j < widget_count; ++j) {
			curr_widget = gfc_list_get_nth(curr->widgets, j);
			if (curr_widget->draw) {
				curr_widget->draw(curr_widget);
			}
		}
	}

}

Window *ui_system_create_window(int draw_layer) {
	int count, i;
	Window *win, *curr;

	// Create the window
	win = window_new();
	win->draw_layer = draw_layer;

	// Find index where window should be appended
	count = gfc_list_count(ui_system);
	for (i = 0; i < count; ++i) {
		curr = gfc_list_get_nth(ui_system, i);
		if (curr->draw_layer <= draw_layer) break;	
	}

	// Append the window
	gfc_list_insert(ui_system, win, i);

	// Return the window object created
	return win;
}

Window *window_new() {
	Window *win = calloc(1, sizeof(Window));
	if (!win) {
		slog("failed to allocate memory for window");
		return NULL;
	}
	return win;
}

void window_free(Window *self) {
	if (!self) return;
	


	free(self);
}

void window_add_widget(Window *self, Widget *widget) {

}

void window_sort_widgets(Window *self) {

}
