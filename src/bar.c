#include "gfc_config.h"
#include "gfc_shape.h"
#include "gfc_vector.h"

#include "gf2d_sprite.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"

#include "entity.h"

#include "ui/bar.h"

typedef struct {
	GFC_Vector2D	box;	// <Slider box
	float 		value;	// <Slider value between 0 and 1
	Entity		*target;
}BarData;

void w_bar_configure(Widget *self, SJson *json) {
	if (!self || !json) return;
	SJson *data_obj = sj_object_get_value(json, "data");

	BarData *data = malloc(sizeof(BarData));
	sj_object_get_vector2d(data_obj, "box", &(data->box));
	data->value = 0;
	self->data = data;

	self->draw = w_bar_draw;
	self->do_draw = 0;
}

void w_bar_draw(Widget *self) {
	if (!self) return;
	BarData *data = (BarData *)self->data;
	if (!data) return;

	GFC_Rect r;
	r.x = self->position.x;
	r.y = self->position.y;
	r.w = data->box.x;
	r.h = data->box.y;
	
	gf2d_draw_rect_filled(r, GFC_COLOR_BLACK);
	r.w = data->box.x * data->value;
	gf2d_draw_rect_filled(r, GFC_COLOR_RED);
}

void w_bar_set_value(Widget *self, float value) {
	if (!self) return;
	BarData *data = (BarData *)self->data;
	if (!data) return;
	data->value = value;
	if (data->value < 0) data->value = 0;
	if (data->value > 1) data->value = 1;
}
