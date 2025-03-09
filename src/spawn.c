#include "simple_logger.h"

#include "bug.h"
#include "player.h"
#include "entity.h"
#include "stage_entity.h"
#include "spawn.h"

static Spawn spawnList[] = 
{
	{
		"player",
		player_spawn
	},
	{
		"bug",
		bug_new_entity
	},
	{
		"temp_platform",
		spawn_temp_platform
	},
	{
		"buzzsaw",
		spawn_buzzsaw
	},
	{
		"turret",
		spawn_turret
	},
	{0}
};

Entity* spawn_entity(const char *name, GFC_Vector2D position, const char *config) {
	Spawn *spawn;
	Entity *ent;
	if (!name) {
		slog("no spawn name provided");
		return NULL;
	}
	for (spawn = spawnList; spawn->name != 0; ++spawn) {
		if (strcmp(name, spawn->name) == 0 ){
			if (spawn->spawn) {
				ent = spawn->spawn(position, config);
				if (ent) {
					return ent;
				}
			}
		}
	}
	
	slog("failed to spawn entity");
	return NULL;
}


