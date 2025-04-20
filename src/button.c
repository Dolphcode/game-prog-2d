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
	{
		level_editor_dec_selection,
		"level_editor_dec_index"
	},
	{
		level_editor_tile_mode,
		"level_editor_tile_mode"
	},
	{
		level_editor_hazard_mode,
		"level_editor_hazard_mode"
	},
	{
		level_editor_enemy_mode,
		"level_editor_enemy_mode"
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
	self->draw = w_button_draw;
	self->do_draw = 1;
}

void w_button_draw(Widget *self) {
	if (!self || !self->sprite) return;

	int frame = 0;
	if (self->clicked) frame = 2;
	else if (self->hovering) frame = 1;

	gf2d_sprite_draw(
		self->sprite,
		self->position,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		(Uint32)frame);
}
