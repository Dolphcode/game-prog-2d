#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "simple_json.h"

#include "gf2d_graphics.h"

#include "ui/window.h"
#include "ui/widget.h"

void w_button_configure(Widget *self, SJson *json);

void w_button_draw(Widget *self);

#endif
