#ifndef __LABEL_H__
#define __LABEL_H__

#include <SDL_ttf.h>

#include "simple_json.h"

#include "gf2d_graphics.h"

#include "ui/window.h"
#include "ui/widget.h"

typedef struct {
	TTF_Font	*font;		// <The font object associated with this label
	SDL_Surface	*surface;	// <The SDL surface associated with this object
	SDL_Texture	*texture;	// <The SDL texture associated with this object
	char		font_path[256];	// <The path to the font
	char		text[1024];	// <The text displayed via this label (must call w_label_set_text to update)
	int		font_size;	// <The size of the font rendered for this label in pts
}W_LabelData;

void w_label_configure(Widget *self, SJson *json);

void w_label_data_free(Widget *self);

void w_label_draw(Widget *self);

/**
 * @brief must call this to set text as this will trigger a re-render of the label
 */
void w_label_set_text(Widget *self, const char *new_text);

#endif
