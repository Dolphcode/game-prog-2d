#include "simple_logger.h"
#include "simple_json.h"

#include "gf2d_draw.h"

#include "ui/window.h"

Uint8 DRAW_UI_BOXES = 0;

/*
typedef struct Window_S {
	Uint8		_active;	// Whether this window is active or not
	GFC_List	*widgets; 	// The list of widgets attached to this window
}Window;
*/

static GFC_List *ui_system = NULL;
static int mouse_caught = 0;

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

int ui_system_mouse_caught() {
	return mouse_caught;
}

void ui_system_update_all() {
	//mouse_caught = 0
	int mouse_caught_one = 0;

	int count, i, widget_count, j, x, y, mouse_down = SDL_GetMouseState(&x, &y);
	Window *curr;
	Widget *curr_widget;
	count = gfc_list_count(ui_system);
	for (i = 0; i < count; ++i) {
		curr = gfc_list_get_nth(ui_system, i);
		//if (!curr->_active) continue;
		
		widget_count = gfc_list_count(curr->widgets);
		for (j = 0; j < widget_count; ++j) {
			curr_widget = gfc_list_get_nth(curr->widgets, j);
			if (!curr_widget || !curr_widget->do_draw) continue;
			

			if (curr_widget->hovering) {
				// If we're still in the box we can check if we just clicked
				if (x > curr_widget->box.x && 
						x < curr_widget->box.x + curr_widget->box.w && 
						y > curr_widget->box.y && 
						y < curr_widget->box.y + curr_widget->box.h) {
					if (curr->_active && !mouse_caught && mouse_down && !curr_widget->clicked) {
						slog("clicked");
						mouse_caught_one = 1;
						curr_widget->clicked = 1;
						if (curr_widget->on_click_ref) curr_widget->on_click_ref(curr_widget);
						if (curr_widget->on_click) curr_widget->on_click(curr_widget);
					}
				} else {
					slog("hover exit");
					curr_widget->hovering = 0;
					if (curr_widget->on_hover_exit) curr_widget->on_hover_exit(curr_widget);
				}
			} else {
				if (curr->_active && x > curr_widget->box.x && 
						x < curr_widget->box.x + curr_widget->box.w && 
						y > curr_widget->box.y && 
						y < curr_widget->box.y + curr_widget->box.h) {
					curr_widget->hovering = 1;
					slog("hover enter");
					if (curr_widget->on_hover_enter) curr_widget->on_hover_enter(curr_widget);
					if (!mouse_caught && mouse_down && !curr_widget->clicked) {
						slog("clicked");
						mouse_caught_one = 1;
						curr_widget->clicked = 1;
						if (curr_widget->on_click_ref) curr_widget->on_click_ref(curr_widget);
						if (curr_widget->on_click) curr_widget->on_click(curr_widget);
					}
				} 
			}

			if (curr_widget->clicked && !mouse_down) {
				slog("released");
				mouse_caught = 0;
				curr_widget->clicked = 0;
				if (curr_widget->on_release) curr_widget->on_release(curr_widget);
			}
		}
	}

	if (mouse_caught_one) mouse_caught = 1;
}

void ui_system_draw_all() {
	int count, i, widget_count, j;
	Window *curr;
	Widget *curr_widget;

	count = gfc_list_count(ui_system);
	for (i = 0; i < count; ++i) {
		curr = gfc_list_get_nth(ui_system, i);
		if (!curr->_active) continue;

		//slog("drawing window with draw_layer %d", curr->draw_layer);
		widget_count = gfc_list_count(curr->widgets);
		for (j = 0; j < widget_count; ++j) {
			curr_widget = gfc_list_get_nth(curr->widgets, j);
			if (curr_widget->do_draw && curr_widget->draw) {
				curr_widget->draw(curr_widget);

				if (DRAW_UI_BOXES) {
					if (curr_widget->clicked) {
						gf2d_draw_rect(curr_widget->box, GFC_COLOR_RED);
					} else if (curr_widget->hovering) {
						gf2d_draw_rect(curr_widget->box, GFC_COLOR_YELLOW);
					} else {
						gf2d_draw_rect(curr_widget->box, GFC_COLOR_GREEN);
					}
				}
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

Window *ui_system_load_window(const char *path) {
	if (!path) {
		slog("No path provided");
		return NULL;
	}

	// Load the json object for this window
	SJson *json = sj_load(path);
	if (!json) {
		slog("failed to load json at path '%s'", path);
		return NULL;
	}

	// Get the draw layer and create the window object
	int draw_layer;
	sj_object_get_int(json, "draw_layer", &draw_layer);
	Window *win = ui_system_create_window(draw_layer);

	// Now copy the name of the window
	const char *name = sj_object_get_string(json, "name");
	strcpy(win->name, name);

	// Now begin creating the widgets
	SJson *widget_list, *widget_obj;
	Widget *curr_widget;
	int widget_count;
	sj_object_get_int(json, "widget_count", &widget_count);
	widget_list = sj_object_get_value(json, "widgets");
	for (int i = 0; i < widget_count; ++i) {
		widget_obj = sj_array_get_nth(widget_list, i);
		if (!widget_obj) continue;

		curr_widget = widget_new();
		widget_configure(curr_widget, widget_obj);
		window_add_widget(win, curr_widget);
	}

	sj_free(json);
	return win;
}

Window *ui_system_get_window(const char *name) {
	Window *curr;
	int count = gfc_list_get_count(ui_system);
	for (int i = 0; i < count; ++i) {
		curr = gfc_list_get_nth(ui_system, i);
		if (!curr)continue;
		if (strcmp(name, curr->name) == 0) {
			slog("found %s", name);
			return curr;
		}
	}
	slog("failed to find window called %s", name);
	return NULL;
}

Window *window_new() {
	Window *win = calloc(1, sizeof(Window));
	if (!win) {
		slog("failed to allocate memory for window");
		return NULL;
	}
	
	// Allocate memory for the widget list
	win->widgets = gfc_list_new();

	return win;
}

void window_free(Window *self) {
	if (!self) return;
	
	// Free the widget list first
	if (self->widgets) {
		int i, count = gfc_list_count(self->widgets);
		Widget *curr = NULL;
		for (i = count - 1; i >= 0; --i) {
			curr = gfc_list_get_nth(self->widgets, i);
			if (curr != NULL) {
				widget_free(curr);
			}
			gfc_list_delete_nth(self->widgets, i);
		}
		gfc_list_delete(self->widgets);
	}

	free(self);
}

void window_add_widget(Window *self, Widget *widget) {
	if (!self || !self->widgets || !widget) return;
	gfc_list_append(self->widgets, widget); // Append the widget
}

void window_sort_widgets(Window *self) {

}

Widget *window_get_widget(Window *self, const char *name) {
	Widget *curr;
	int count = gfc_list_get_count(self->widgets);
	for (int i = 0; i < count; ++i) {
		curr = gfc_list_get_nth(self->widgets, i);
		if (!curr)continue;
		if (strcmp(name, curr->name) == 0) {
			return curr;
		}
	}
	return NULL;
}
