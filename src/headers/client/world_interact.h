#ifndef CRAFT_WORLD_INTERACT_H
#define CRAFT_WORLD_INTERACT_H

#include <math.h>
#include <string.h>

#include "noise.h"

#include "client.h"
#include "config.h"
#include "cube.h"
#include "item.h"
#include "map.h"
#include "sign.h"
#include "db.h"
#include "util.h"
#include "world.h"

#include "common/data_structures.h"
#include "common/model.h"

int chunked(float x);
void force_chunks(Player *player);
Chunk *find_chunk(int p, int q);
void gen_chunk_buffer(Chunk *chunk);
void compute_chunk(WorkerItem *item);
void generate_chunk(Chunk *chunk, WorkerItem *item);
void request_chunk(int p, int q);
void init_chunk(Chunk *chunk, int p, int q);
void load_chunk(WorkerItem *item);
void dirty_chunk(Chunk *chunk);
int _gen_sign_buffer(
		GLfloat *data, float x, float y, float z, int face, const char *text);
int highest_block(float x, float z);
void unset_sign(int x, int y, int z);
void unset_sign_face(int x, int y, int z, int face);
void _set_sign(
		int p, int q, int x, int y, int z, int face, const char *text, int dirty);
void set_sign(int x, int y, int z, int face, const char *text);
void toggle_light(int x, int y, int z) ;
void set_light(int p, int q, int x, int y, int z, int w);
void _set_block(int p, int q, int x, int y, int z, int w, int dirty);
void set_block(int x, int y, int z, int w);
int player_intersects_block(
		int height,
		float x, float y, float z,
		int hx, int hy, int hz);
Player *find_player(int id);
void update_player(Player *player,
				   float x, float y, float z, float rx, float ry, int interpolate);
void delete_player(int id);

#endif
