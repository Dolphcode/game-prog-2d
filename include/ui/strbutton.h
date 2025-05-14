#ifndef __STRBUTTON_H__
#define __STRBUTTON_H__

#include "simple_json.h"

#include "gf2d_graphics.h"

#include "ui/window.h"
#include "ui/widget.h"

void w_strbutton_configure(Widget *self, SJson *json);

void w_strbutton_draw(Widget *self);

#endif
