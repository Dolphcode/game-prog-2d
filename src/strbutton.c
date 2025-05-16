#include "simple_json.h"
#include "simple_logger.h"

#include "editor/level_editor.h"
#include "game_manager.h"

#include "ui/strbutton.h"

typedef struct {
	void	(*on_click)(const char *);	// <The associated callback function
	char	*id;				// <The identifier for this callback function
}CallbackEntry;

void w_strbutton_print(const char *);

static CallbackEntry callback_list[] = {
	{
		game_manager_bindkey,
		"game_manager_bindkey"
	},
	{
		load_level_param,
		"load_level_param"
	},
	{
		w_strbutton_print,
		"strbutton_print"
	},
	{0}
};

typedef struct {
	void	(*on_click)(const char *);
	char	param[256];
}StrButtonData;

void w_strbutton_click(Widget *self) {
	if (!self) return;
	StrButtonData *data = (StrButtonData *)self->data;
	if (!data) return;
	data->on_click(data->param);
}

void w_strbutton_configure(Widget *self, SJson *json) {
	if (!self || !json) return;
	SJson *callback_data = sj_object_get_value(json, "data");
	const char *callback_id = sj_object_get_string(callback_data, "callback");
	const char *param_data = sj_object_get_string(callback_data, "param");
	for (CallbackEntry *ptr = callback_list; ptr->on_click != NULL; ptr++) {
		if (strcmp(ptr->id, callback_id) == 0) {
			StrButtonData *data = malloc(sizeof(StrButtonData));
			self->data = data;
			data->on_click = ptr->on_click;
			strcpy(data->param, param_data);
			self->on_click_ref = w_strbutton_click;
		}
	}
	self->draw = w_strbutton_draw;
	self->do_draw = 1;
}

void w_strbutton_draw(Widget *self) {
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

void w_strbutton_print(const char *out) {
	slog("%s", out);
}
