#include "simple_logger.h"

#include "body.h"

Body *body_new() {
	// Allocate memory
	Body *body;
	body = gfc_allocate_array(sizeof(Body), 1);
	if (!body) {
		slog("failed to allocate memory for physics body");
		return NULL;
	}

	return body;
}

void body_free(Body *self) {
	if (!self) return;
	slog ("freeing the body");
	free(self);
}
