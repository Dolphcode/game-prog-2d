#include "simple_json.h"

#include "editor/level_editor.h"

#include "ui/button.h"

typedef struct {
	void	(*on_click)();			// <The associated callback function
	char	*id;				// <The identifier for this callback function
}CallbackEntry;

static CallbackEntry callback_list[] = {
	{
		level_editor_inc_selection,
		"level_editor_inc_index"
	},
	{0}
};

void w_button_configure(Widget *self, SJson *json) {
	if (!self || !json) return;
	SJson *callback_data = sj_object_get_value(json, "data");
	const char *callback_id = sj_object_get_string(callback_data, "callback");
	for (CallbackEntry *ptr = callback_list; ptr->on_click != NULL; ptr++) {
		if (strcmp(ptr->id, callback_id) == 0) {
			self->on_click = ptr->on_click;
		}
	}

	self->do_draw = 1;
}

void w_button_draw(Widget *self) {

}
