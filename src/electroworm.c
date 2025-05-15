#include "simple_logger.h"
#include "simple_json.h"

#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

#include "projectile.h"
#include "world.h"
#include "camera.h"
#include "boss.h"

#include "ui/window.h"
#include "ui/widget.h"
#include "ui/bar.h"

#define WORM_FRAME_DELAY 10

typedef struct WormSegment_S {
	int			index;
	GFC_Vector2D		current_action;
	GFC_Vector2D		actions[WORM_FRAME_DELAY];
	struct WormSegment_S	*next;	// <The next segment to pass actions to
	float	time;		// <timer for some behavior
	Entity	*head;		// <A reference to the head
	Entity	*self;		// <A reference to the entity itself
	Widget 	*bossbar;	// <A reference to the bossbar (only head needs it)
}WormSegmentData;

void electroworm_segment_pass_action(WormSegmentData *self, GFC_Vector2D action) {
	if (!self) return;
	gfc_vector2d_copy(self->current_action, self->actions[self->index]);
	gfc_vector2d_copy(self->actions[self->index], action);
	(self->index)++;
	if (self->index >= WORM_FRAME_DELAY) self->index = 0;
       	if (self->next != NULL) {
		electroworm_segment_pass_action(self->next, self->current_action);
       	}	       
}

void electroworm_head_damage(Entity *self, float damage) {
	if (!self || self->i_time > 0) return;
	WormSegmentData *data = (WormSegmentData *)self->data;
	if (!data) return;
	self->i_time = self->immunity;
	self->health -= damage;
	//slog("hit!");
	if (self->health <= 0) {
		data->bossbar->do_draw = 0;
		self->alive = 0;
		self->body->disabled = 1;
		data = data->next;
		while (data != NULL) {
			data->self->alive = 0;
			data->self->body->disabled = 1;
			data = data->next;
		}
	}
}

void electroworm_body_damage(Entity *self, float damage) {
	if (!self) return;
	WormSegmentData *data = (WormSegmentData *)self->data;
	if (!data) return;
	if (data->head) {
		data->head->damage(data->head, damage * 0.5);
	}
}

void electroworm_body_update(Entity *self) {
	if (!self) return;
	WormSegmentData *data = (WormSegmentData *)self->data;
	if (!data) return;

	data->time -= 0.1;
}

void electroworm_body_think(Entity *self) {
	if (!self) return;
	WormSegmentData *data = (WormSegmentData *)self->data;
	if (!data) return;
	gfc_vector2d_copy(self->velocity, data->current_action);

	GFC_Vector2D up, down;
	if (data->time <= 0) {
		data->time = 10;
		up = gfc_vector2d_rotate(self->velocity, -M_PI * 0.5);
		down = gfc_vector2d_rotate(self->velocity, -M_PI * 0.5);
		gfc_vector2d_normalize(&up);
		gfc_vector2d_normalize(&down);

		projectile_fire_ex("turret_shot", self->position, up, 800, 0, 0, 1);
		projectile_fire_ex("turret_shot", self->position, down, 800, 0, 0, 1);
	}
}

void electroworm_head_update(Entity *self) {
	if (!self) return;
	WormSegmentData *data = (WormSegmentData *)self->data;
	if (!data) return;	
	self->i_time -= 1;	
	w_bar_set_value(data->bossbar, self->health / self->max_health);

	electroworm_segment_pass_action(data->next, data->actions[data->index]);	
	data->actions[data->index] = data->current_action;
	(data->index)++;
	if (data->index >= WORM_FRAME_DELAY) data->index = 0;
}

void electroworm_head_think(Entity *self) {
	if (!self) return;
	WormSegmentData *data = (WormSegmentData *)self->data;
	if (!data) return;
	
	// Get the player reference
	Entity *player_ref = world_get_active()->player;
	
	if (gfc_vector2d_magnitude(self->velocity) > 500) {
		gfc_vector2d_set_magnitude(&self->velocity, 500);
	}

	GFC_Vector2D direction;

	gfc_vector2d_sub(direction, player_ref->position, self->position);
	gfc_vector2d_normalize(&direction);
	gfc_vector2d_scale_by(direction, direction, gfc_vector2d(700, 700));

	gfc_vector2d_add(self->acceleration, self->acceleration, direction);
	
	gfc_vector2d_copy(data->current_action, self->velocity);
}

void electroworm_draw(Entity *self) {
	// Verify pointers
	if (!self || !self->sprite) return;

	// Calculate draw position and scale
	GFC_Vector2D scale = main_camera_get_zoom();

	GFC_Vector2D draw_pos = main_camera_calc_drawpos(self->position);

	GFC_Vector2D center = self->sprite_offset;

	float rot = gfc_vector2d_angle(self->velocity) * 180 / M_PI + 90.0;
	
	// Draw the sprite
	gf2d_sprite_draw(
		self->sprite,
		draw_pos,
		&scale,
		&center,
		&rot,
		NULL,
		NULL,
		(Uint32)self->frame);

	// Draw the point
	if (DRAW_CENTER) gf2d_draw_circle(draw_pos, 4, GFC_COLOR_LIGHTGREEN);
}

Entity* electroworm_spawn(GFC_Vector2D position, const char *config) {
	Entity *curr_segment, *next_segment;
	WormSegmentData *curr_data, *next_data;

	// Spawn the tail
	curr_segment = entity_new();
	if (!curr_segment) {
		slog("failed to allocate memory for electroworm tail");
		return NULL;
	}
	entity_configure_from_file(curr_segment, "def/boss/electroworm_tail.def");	
	curr_data = malloc(sizeof(WormSegmentData));
	memset(curr_data, 0, sizeof(WormSegmentData));
	curr_segment->data = curr_data;
	curr_segment->think = electroworm_body_think;
	curr_segment->draw = electroworm_draw;
	curr_segment->damage = electroworm_body_damage;
	curr_segment->update = electroworm_body_update;
	curr_data->time = 10;
	curr_segment->can_grapple = 1;
	curr_segment->frame = 2;
	curr_segment->alive = 1;
	curr_data->self = curr_segment;
	gfc_vector2d_copy(curr_segment->position, position);
	
	// Now spawn the segments
	for (int i = 0; i < 10; ++i) {
			next_segment = entity_new();
			if (!next_segment) {
				slog("failed to allocate memory for electroworm segment %d", i);
				return NULL;
			}
			entity_configure_from_file(next_segment, "def/boss/electroworm_body.def");	
			next_data = malloc(sizeof(WormSegmentData));
			memset(next_data, 0, sizeof(WormSegmentData));
			next_segment->data = next_data;
			next_segment->think = electroworm_body_think;
			next_segment->draw = electroworm_draw;
			next_segment->damage = electroworm_body_damage;
			next_segment->update = electroworm_body_update;
			next_segment->alive = 1;
			next_data->time = 10;
			gfc_vector2d_copy(next_segment->position, position);

			next_segment->frame = 1;
			next_data->self = next_segment;
			next_data->next = curr_data;

			curr_data = next_data;
			curr_segment = next_segment;
	}
	
	next_segment = entity_new();
	if (!next_segment){
		slog("failed to allocate memory for the electroworm head");       
		return NULL;
	}
	entity_configure_from_file(next_segment, "def/boss/electroworm_head.def");
	
	next_data = malloc(sizeof(WormSegmentData));
	memset(next_data, 0, sizeof(WormSegmentData));
	next_segment->data = next_data;
	next_segment->think = electroworm_head_think;
	next_segment->update = electroworm_head_update;
	next_segment->draw = electroworm_draw;
	next_segment->damage = electroworm_head_damage;
	next_segment->alive = 1;

	// Get the bossbar
	Window *win = ui_system_get_window("player_hud");
	Widget *boss_bar = window_get_widget(win, "boss_bar");
	next_data->bossbar = boss_bar;
	next_data->bossbar->do_draw = 1;

	gfc_vector2d_copy(next_segment->position, position);
	
	next_data->next = curr_data;

	// Set the head
	WormSegmentData *curr_parse_data = next_data;
	while (curr_parse_data != NULL) {
		curr_parse_data->head = next_segment;
		curr_parse_data = curr_parse_data->next;
	}

	curr_segment = next_segment;
	curr_data = next_data;
	slog("Everything has been made");
	return curr_segment;

}
