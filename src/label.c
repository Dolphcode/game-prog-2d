#include "simple_logger.h"

#include "gf2d_graphics.h"

#include "ui/label.h"

/*
typedef struct {
	TTF_Font	*font;		// <The font object associated with this label
	SDL_Surface	*surface;	// <The SDL surface associated with this object
	SDL_Texture	*texture;	// <The SDL texture associated with this object
	const char	*text;		// <The text displayed via this label (must call w_label_set_text to update)
	float		font_size;	// <The size of the font rendered for this label in pts
}W_LabelData;*/

void w_label_load_font(W_LabelData *data) {
	if (!data) return;

	// First free the font if it exists
	if (data->font != NULL) {
		TTF_CloseFont(data->font);
		data->font = NULL;
	}

	// Now begin loading the font from a path
	data->font = TTF_OpenFont(data->font_path, data->font_size);
	if (!data->font) {
		slog("failed to open font");
		return;
	}
}

void w_label_render_font(W_LabelData *data) {
	if (!data) return;

	// First free the surface and textures if they exist
	if (data->surface != NULL) {
		SDL_FreeSurface(data->surface);
		data->surface = NULL;
	}

	if (data->texture != NULL) {
		SDL_DestroyTexture(data->texture);
		data->texture = NULL;
	}

	// Now begin rendering
	data->surface = TTF_RenderUTF8_Blended_Wrapped(data->font, data->text, gfc_color_to_sdl(GFC_COLOR_WHITE), 0);
	if (!data->surface) {
		slog("Failed to create label surface");
		return;
	}
	data->surface = gf2d_graphics_screen_convert(&(data->surface));
	
	data->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), data->surface);
	if (!data->texture) {
		slog("Failed to create the label texture");
		return;
	}
}

void w_label_configure(Widget *self, SJson *json) {
	if (!self || !json) return;

	// Allocate memory for the label data
	W_LabelData *data = calloc(1, sizeof(W_LabelData));
	self->data = data;

	// Load from the json object
	SJson *data_json = sj_object_get_value(json, "data");
	if (!data_json) {
		slog("No data member found to load data from");
		return;
	}

	// Load the label's default text
	const char *text = sj_object_get_string(data_json, "text");
	strcpy(data->text, text);

	// Load the label's font size
	sj_object_get_int(data_json, "font_size", &(data->font_size));

	// Now load the font path and render the font
	const char *font_path = sj_object_get_string(data_json, "font");
	strcpy(data->font_path, font_path);
	w_label_load_font(data);
	w_label_render_font(data);

	// Set the draw function and enable drawing by default
	self->draw = w_label_draw;
	self->do_draw = 1;
}

void w_label_data_free(Widget *self) {
	if (!self || !self->data) return;
	W_LabelData *data = (W_LabelData *)self->data;
	
	if (data->font) TTF_CloseFont(data->font);
	if (data->surface) SDL_FreeSurface(data->surface);
	if (data->texture) SDL_DestroyTexture(data->texture);

	free(self->data);
}

void w_label_draw(Widget *self) {
	if (!self) return;
	W_LabelData *data = (W_LabelData *)self->data;
	if (!data) return;

	SDL_Rect rect;
	rect.x = self->position.x;
	rect.y = self->position.y;
	rect.w = data->surface->w;
	rect.h = data->surface->h;

	SDL_RenderCopy(gf2d_graphics_get_renderer(),
			data->texture,
			NULL,
			&rect);

	SDL_SetTextureColorMod(data->texture,
			self->color.r,
			self->color.g,
			self->color.b);
			
}

/**
 * @brief must call this to set text as this will trigger a re-render of the label
 */
void w_label_set_text(Widget *self, const char *new_text) {
	if (!self || !new_text) return;
	W_LabelData *data = (W_LabelData *)self->data;
	if (!data) return;

	strcpy(data->text, new_text);
	w_label_render_font(data); // Call re-render
}
