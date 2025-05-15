#ifndef __BAR_H__
#define __BAR_H__

#include "simple_json.h"

#include "gf2d_graphics.h"

#include "ui/window.h"
#include "ui/widget.h"

void w_bar_configure(Widget *self, SJson *json);

void w_bar_draw(Widget *self);

void w_bar_set_value(Widget *self, float value);

#endif
