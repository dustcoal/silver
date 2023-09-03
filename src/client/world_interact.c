#include <stdlib.h>

#include "client/world_interact.h"

#define XZ_SIZE (CHUNK_SIZE * 3 + 2)
#define XZ_LO (CHUNK_SIZE)
#define XZ_HI (CHUNK_SIZE * 2 + 1)
#define Y_SIZE 258
#define XYZ(x, y, z) ((y) * XZ_SIZE * XZ_SIZE + (x) * XZ_SIZE + (z))
#define XZ(x, z) ((x) * XZ_SIZE + (z))

int has_lights(Chunk *chunk) {
	if (!SHOW_LIGHTS) {
		return 0;
	}
	for (int dp = -1; dp <= 1; dp++) {
		for (int dq = -1; dq <= 1; dq++) {
			Chunk *other = chunk;
			if (dp || dq) {
				other = find_chunk(chunk->p + dp, chunk->q + dq);
			}
			if (!other) {
				continue;
			}
			Map *map = &other->lights;
			if (map->size) {
				return 1;
			}
		}
	}
	return 0;
}

void dirty_chunk(Chunk *chunk) {
	chunk->dirty = 1;
	if (has_lights(chunk)) {
		for (int dp = -1; dp <= 1; dp++) {
			for (int dq = -1; dq <= 1; dq++) {
				Chunk *other = find_chunk(chunk->p + dp, chunk->q + dq);
				if (other) {
					other->dirty = 1;
				}
			}
		}
	}
}

void init_chunk(Chunk *chunk, int p, int q) {
	chunk->p = p;
	chunk->q = q;
	chunk->faces = 0;
	chunk->sign_faces = 0;
	chunk->buffer = 0;
	chunk->sign_buffer = 0;
	dirty_chunk(chunk);
	SignList *signs = &chunk->signs;
	sign_list_alloc(signs, 16);
	db_load_signs(signs, p, q);
	Map *block_map = &chunk->map;
	Map *light_map = &chunk->lights;
	int dx = p * CHUNK_SIZE - 1;
	int dy = 0;
	int dz = q * CHUNK_SIZE - 1;
	map_alloc(block_map, dx, dy, dz, 0x7fff);
	map_alloc(light_map, dx, dy, dz, 0xf);
}

static void map_set_func(int x, int y, int z, int w, void *arg) {
	Map *map = (Map *)arg;
	map_set(map, x, y, z, w);
}

void load_chunk(WorkerItem *item) {
	int p = item->p;
	int q = item->q;
	Map *block_map = item->block_maps[1][1];
	Map *light_map = item->light_maps[1][1];
	create_world(p, q, map_set_func, block_map);
	db_load_blocks(block_map, p, q);
	db_load_lights(light_map, p, q);
}

void request_chunk(int p, int q) {
	int key = db_get_key(p, q);
	client_chunk(p, q, key);
}

void create_chunk(Chunk *chunk, int p, int q) {
	init_chunk(chunk, p, q);

	WorkerItem _item;
	WorkerItem *item = &_item;
	item->p = chunk->p;
	item->q = chunk->q;
	item->block_maps[1][1] = &chunk->map;
	item->light_maps[1][1] = &chunk->lights;
	load_chunk(item);

	request_chunk(p, q);
}

int chunked(float x) {
	return floorf(roundf(x) / CHUNK_SIZE);
}

Chunk *find_chunk(int p, int q) {
	for (int i = 0; i < g->chunk_count; i++) {
		Chunk *chunk = g->chunks + i;
		if (chunk->p == p && chunk->q == q) {
			return chunk;
		}
	}
	return 0;
}

void gen_chunk_buffer(Chunk *chunk) {
	WorkerItem _item;
	WorkerItem *item = &_item;
	item->p = chunk->p;
	item->q = chunk->q;
	for (int dp = -1; dp <= 1; dp++) {
		for (int dq = -1; dq <= 1; dq++) {
			Chunk *other = chunk;
			if (dp || dq) {
				other = find_chunk(chunk->p + dp, chunk->q + dq);
			}
			if (other) {
				item->block_maps[dp + 1][dq + 1] = &other->map;
				item->light_maps[dp + 1][dq + 1] = &other->lights;
			}
			else {
				item->block_maps[dp + 1][dq + 1] = 0;
				item->light_maps[dp + 1][dq + 1] = 0;
			}
		}
	}
	compute_chunk(item);
	generate_chunk(chunk, item);
	chunk->dirty = 0;
}

void force_chunks(Player *player) {
	State *s = &player->state;
	int p = chunked(s->x);
	int q = chunked(s->z);
	int r = 1;
	for (int dp = -r; dp <= r; dp++) {
		for (int dq = -r; dq <= r; dq++) {
			int a = p + dp;
			int b = q + dq;
			Chunk *chunk = find_chunk(a, b);
			if (chunk) {
				if (chunk->dirty) {
					gen_chunk_buffer(chunk);
				}
			}
			else if (g->chunk_count < MAX_CHUNKS) {
				chunk = g->chunks + g->chunk_count++;
				create_chunk(chunk, a, b);
				gen_chunk_buffer(chunk);
			}
		}
	}
}

void occlusion(
		char neighbors[27], char lights[27], float shades[27],
		float ao[6][4], float light[6][4])
{
	static const int lookup3[6][4][3] = {
			{{0, 1, 3}, {2, 1, 5}, {6, 3, 7}, {8, 5, 7}},
			{{18, 19, 21}, {20, 19, 23}, {24, 21, 25}, {26, 23, 25}},
			{{6, 7, 15}, {8, 7, 17}, {24, 15, 25}, {26, 17, 25}},
			{{0, 1, 9}, {2, 1, 11}, {18, 9, 19}, {20, 11, 19}},
			{{0, 3, 9}, {6, 3, 15}, {18, 9, 21}, {24, 15, 21}},
			{{2, 5, 11}, {8, 5, 17}, {20, 11, 23}, {26, 17, 23}}
	};
	static const int lookup4[6][4][4] = {
			{{0, 1, 3, 4}, {1, 2, 4, 5}, {3, 4, 6, 7}, {4, 5, 7, 8}},
			{{18, 19, 21, 22}, {19, 20, 22, 23}, {21, 22, 24, 25}, {22, 23, 25, 26}},
			{{6, 7, 15, 16}, {7, 8, 16, 17}, {15, 16, 24, 25}, {16, 17, 25, 26}},
			{{0, 1, 9, 10}, {1, 2, 10, 11}, {9, 10, 18, 19}, {10, 11, 19, 20}},
			{{0, 3, 9, 12}, {3, 6, 12, 15}, {9, 12, 18, 21}, {12, 15, 21, 24}},
			{{2, 5, 11, 14}, {5, 8, 14, 17}, {11, 14, 20, 23}, {14, 17, 23, 26}}
	};
	static const float curve[4] = {0.0, 0.25, 0.5, 0.75};
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 4; j++) {
			int corner = neighbors[lookup3[i][j][0]];
			int side1 = neighbors[lookup3[i][j][1]];
			int side2 = neighbors[lookup3[i][j][2]];
			int value = side1 && side2 ? 3 : corner + side1 + side2;
			float shade_sum = 0;
			float light_sum = 0;
			int is_light = lights[13] == 15;
			for (int k = 0; k < 4; k++) {
				shade_sum += shades[lookup4[i][j][k]];
				light_sum += lights[lookup4[i][j][k]];
			}
			if (is_light) {
				light_sum = 15 * 4 * 10;
			}
			float total = curve[value] + shade_sum / 4.0;
			ao[i][j] = MIN(total, 1.0);
			light[i][j] = light_sum / 15.0 / 4.0;
		}
	}
}

void light_fill(
		char *opaque, char *light,
		int x, int y, int z, int w, int force)
{
	if (x + w < XZ_LO || z + w < XZ_LO) {
		return;
	}
	if (x - w > XZ_HI || z - w > XZ_HI) {
		return;
	}
	if (y < 0 || y >= Y_SIZE) {
		return;
	}
	if (light[XYZ(x, y, z)] >= w) {
		return;
	}
	if (!force && opaque[XYZ(x, y, z)]) {
		return;
	}
	light[XYZ(x, y, z)] = w--;
	light_fill(opaque, light, x - 1, y, z, w, 0);
	light_fill(opaque, light, x + 1, y, z, w, 0);
	light_fill(opaque, light, x, y - 1, z, w, 0);
	light_fill(opaque, light, x, y + 1, z, w, 0);
	light_fill(opaque, light, x, y, z - 1, w, 0);
	light_fill(opaque, light, x, y, z + 1, w, 0);
}

void compute_chunk(WorkerItem *item) {
	char *opaque = (char *)calloc(XZ_SIZE * XZ_SIZE * Y_SIZE, sizeof(char));
	char *light = (char *)calloc(XZ_SIZE * XZ_SIZE * Y_SIZE, sizeof(char));
	char *highest = (char *)calloc(XZ_SIZE * XZ_SIZE, sizeof(char));

	int ox = item->p * CHUNK_SIZE - CHUNK_SIZE - 1;
	int oy = -1;
	int oz = item->q * CHUNK_SIZE - CHUNK_SIZE - 1;

	// check for lights
	int has_light = 0;
	if (SHOW_LIGHTS) {
		for (int a = 0; a < 3; a++) {
			for (int b = 0; b < 3; b++) {
				Map *map = item->light_maps[a][b];
				if (map && map->size) {
					has_light = 1;
				}
			}
		}
	}

	// populate opaque array
	for (int a = 0; a < 3; a++) {
		for (int b = 0; b < 3; b++) {
			Map *map = item->block_maps[a][b];
			if (!map) {
				continue;
			}
			MAP_FOR_EACH(map, ex, ey, ez, ew) {
				int x = ex - ox;
				int y = ey - oy;
				int z = ez - oz;
				int w = ew;
				// TODO: this should be unnecessary
				if (x < 0 || y < 0 || z < 0) {
					continue;
				}
				if (x >= XZ_SIZE || y >= Y_SIZE || z >= XZ_SIZE) {
					continue;
				}
				// END TODO
				opaque[XYZ(x, y, z)] = !is_transparent(w);
				if (opaque[XYZ(x, y, z)]) {
					highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
				}
			} END_MAP_FOR_EACH;
		}
	}

	// flood fill light intensities
	if (has_light) {
		for (int a = 0; a < 3; a++) {
			for (int b = 0; b < 3; b++) {
				Map *map = item->light_maps[a][b];
				if (!map) {
					continue;
				}
				MAP_FOR_EACH(map, ex, ey, ez, ew) {
					int x = ex - ox;
					int y = ey - oy;
					int z = ez - oz;
					light_fill(opaque, light, x, y, z, ew, 1);
				} END_MAP_FOR_EACH;
			}
		}
	}

	Map *map = item->block_maps[1][1];

	// count exposed faces
	int miny = 256;
	int maxy = 0;
	int faces = 0;
	MAP_FOR_EACH(map, ex, ey, ez, ew) {
		if (ew <= 0) {
			continue;
		}
		int x = ex - ox;
		int y = ey - oy;
		int z = ez - oz;
		int f1 = !opaque[XYZ(x - 1, y, z)];
		int f2 = !opaque[XYZ(x + 1, y, z)];
		int f3 = !opaque[XYZ(x, y + 1, z)];
		int f4 = !opaque[XYZ(x, y - 1, z)] && (ey > 0);
		int f5 = !opaque[XYZ(x, y, z - 1)];
		int f6 = !opaque[XYZ(x, y, z + 1)];
		int total = f1 + f2 + f3 + f4 + f5 + f6;
		if (total == 0) {
			continue;
		}
		if (is_plant(ew)) {
			total = 4;
		}
		miny = MIN(miny, ey);
		maxy = MAX(maxy, ey);
		faces += total;
	} END_MAP_FOR_EACH;

	// generate geometry
	GLfloat *data = malloc_faces(10, faces);
	int offset = 0;
	MAP_FOR_EACH(map, ex, ey, ez, ew) {
		if (ew <= 0) {
			continue;
		}
		int x = ex - ox;
		int y = ey - oy;
		int z = ez - oz;
		int f1 = !opaque[XYZ(x - 1, y, z)];
		int f2 = !opaque[XYZ(x + 1, y, z)];
		int f3 = !opaque[XYZ(x, y + 1, z)];
		int f4 = !opaque[XYZ(x, y - 1, z)] && (ey > 0);
		int f5 = !opaque[XYZ(x, y, z - 1)];
		int f6 = !opaque[XYZ(x, y, z + 1)];
		int total = f1 + f2 + f3 + f4 + f5 + f6;
		if (total == 0) {
			continue;
		}
		char neighbors[27] = {0};
		char lights[27] = {0};
		float shades[27] = {0};
		int index = 0;
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				for (int dz = -1; dz <= 1; dz++) {
					neighbors[index] = opaque[XYZ(x + dx, y + dy, z + dz)];
					lights[index] = light[XYZ(x + dx, y + dy, z + dz)];
					shades[index] = 0;
					if (y + dy <= highest[XZ(x + dx, z + dz)]) {
						for (int oy = 0; oy < 8; oy++) {
							if (opaque[XYZ(x + dx, y + dy + oy, z + dz)]) {
								shades[index] = 1.0 - oy * 0.125;
								break;
							}
						}
					}
					index++;
				}
			}
		}
		float ao[6][4];
		float light[6][4];
		occlusion(neighbors, lights, shades, ao, light);
		if (is_plant(ew)) {
			total = 4;
			float min_ao = 1;
			float max_light = 0;
			for (int a = 0; a < 6; a++) {
				for (int b = 0; b < 4; b++) {
					min_ao = MIN(min_ao, ao[a][b]);
					max_light = MAX(max_light, light[a][b]);
				}
			}
			float rotation = simplex2(ex, ez, 4, 0.5, 2) * 360;
			make_plant(
					data + offset, min_ao, max_light,
					ex, ey, ez, 0.5, ew, rotation);
		}
		else {
			make_cube(
					data + offset, ao, light,
					f1, f2, f3, f4, f5, f6,
					ex, ey, ez, 0.5, ew);
		}
		offset += total * 60;
	} END_MAP_FOR_EACH;

	free(opaque);
	free(light);
	free(highest);

	item->miny = miny;
	item->maxy = maxy;
	item->faces = faces;
	item->data = data;
}

int _gen_sign_buffer(
		GLfloat *data, float x, float y, float z, int face, const char *text)
{
	static const int glyph_dx[8] = {0, 0, -1, 1, 1, 0, -1, 0};
	static const int glyph_dz[8] = {1, -1, 0, 0, 0, -1, 0, 1};
	static const int line_dx[8] = {0, 0, 0, 0, 0, 1, 0, -1};
	static const int line_dy[8] = {-1, -1, -1, -1, 0, 0, 0, 0};
	static const int line_dz[8] = {0, 0, 0, 0, 1, 0, -1, 0};
	if (face < 0 || face >= 8) {
		return 0;
	}
	int count = 0;
	float max_width = 64;
	float line_height = 1.25;
	char lines[1024];
	int rows = wrap(text, max_width, lines, 1024);
	rows = MIN(rows, 5);
	int dx = glyph_dx[face];
	int dz = glyph_dz[face];
	int ldx = line_dx[face];
	int ldy = line_dy[face];
	int ldz = line_dz[face];
	float n = 1.0 / (max_width / 10);
	float sx = x - n * (rows - 1) * (line_height / 2) * ldx;
	float sy = y - n * (rows - 1) * (line_height / 2) * ldy;
	float sz = z - n * (rows - 1) * (line_height / 2) * ldz;
	char *key;
	char *line = tokenize(lines, "\n", &key);
	while (line) {
		int length = strlen(line);
		int line_width = string_width(line);
		line_width = MIN(line_width, max_width);
		float rx = sx - dx * line_width / max_width / 2;
		float ry = sy;
		float rz = sz - dz * line_width / max_width / 2;
		for (int i = 0; i < length; i++) {
			int width = char_width(line[i]);
			line_width -= width;
			if (line_width < 0) {
				break;
			}
			rx += dx * width / max_width / 2;
			rz += dz * width / max_width / 2;
			if (line[i] != ' ') {
				make_character_3d(
						data + count * 30, rx, ry, rz, n / 2, face, line[i]);
				count++;
			}
			rx += dx * width / max_width / 2;
			rz += dz * width / max_width / 2;
		}
		sx += n * line_height * ldx;
		sy += n * line_height * ldy;
		sz += n * line_height * ldz;
		line = tokenize(NULL, "\n", &key);
		rows--;
		if (rows <= 0) {
			break;
		}
	}
	return count;
}

void gen_sign_buffer(Chunk *chunk) {
	SignList *signs = &chunk->signs;

	// first pass - count characters
	int max_faces = 0;
	for (int i = 0; i < signs->size; i++) {
		Sign *e = signs->data + i;
		max_faces += strlen(e->text);
	}

	// second pass - generate geometry
	GLfloat *data = malloc_faces(5, max_faces);
	int faces = 0;
	for (int i = 0; i < signs->size; i++) {
		Sign *e = signs->data + i;
		faces += _gen_sign_buffer(
				data + faces * 30, e->x, e->y, e->z, e->face, e->text);
	}

	del_buffer(chunk->sign_buffer);
	chunk->sign_buffer = gen_faces(5, faces, data);
	chunk->sign_faces = faces;
}

void generate_chunk(Chunk *chunk, WorkerItem *item) {
	chunk->miny = item->miny;
	chunk->maxy = item->maxy;
	chunk->faces = item->faces;
	del_buffer(chunk->buffer);
	chunk->buffer = gen_faces(10, item->faces, item->data);
	gen_sign_buffer(chunk);
}

int highest_block(float x, float z) {
	int result = -1;
	int nx = roundf(x);
	int nz = roundf(z);
	int p = chunked(x);
	int q = chunked(z);
	Chunk *chunk = find_chunk(p, q);
	if (chunk) {
		Map *map = &chunk->map;
		MAP_FOR_EACH(map, ex, ey, ez, ew) {
				if (is_obstacle(ew) && ex == nx && ez == nz) {
					result = MAX(result, ey);
				}
			} END_MAP_FOR_EACH;
	}
	return result;
}

void unset_sign(int x, int y, int z) {
	int p = chunked(x);
	int q = chunked(z);
	Chunk *chunk = find_chunk(p, q);
	if (chunk) {
		SignList *signs = &chunk->signs;
		if (sign_list_remove_all(signs, x, y, z)) {
			chunk->dirty = 1;
			db_delete_signs(x, y, z);
		}
	}
	else {
		db_delete_signs(x, y, z);
	}
}

void unset_sign_face(int x, int y, int z, int face) {
	int p = chunked(x);
	int q = chunked(z);
	Chunk *chunk = find_chunk(p, q);
	if (chunk) {
		SignList *signs = &chunk->signs;
		if (sign_list_remove(signs, x, y, z, face)) {
			chunk->dirty = 1;
			db_delete_sign(x, y, z, face);
		}
	}
	else {
		db_delete_sign(x, y, z, face);
	}
}

void _set_sign(
		int p, int q, int x, int y, int z, int face, const char *text, int dirty)
{
	if (strlen(text) == 0) {
		unset_sign_face(x, y, z, face);
		return;
	}
	Chunk *chunk = find_chunk(p, q);
	if (chunk) {
		SignList *signs = &chunk->signs;
		sign_list_add(signs, x, y, z, face, text);
		if (dirty) {
			chunk->dirty = 1;
		}
	}
	db_insert_sign(p, q, x, y, z, face, text);
}

void set_sign(int x, int y, int z, int face, const char *text) {
	int p = chunked(x);
	int q = chunked(z);
	_set_sign(p, q, x, y, z, face, text, 1);
	client_sign(x, y, z, face, text);
}

void toggle_light(int x, int y, int z) {
	int p = chunked(x);
	int q = chunked(z);
	Chunk *chunk = find_chunk(p, q);
	if (chunk) {
		Map *map = &chunk->lights;
		int w = map_get(map, x, y, z) ? 0 : 15;
		map_set(map, x, y, z, w);
		db_insert_light(p, q, x, y, z, w);
		client_light(x, y, z, w);
		dirty_chunk(chunk);
	}
}

void set_light(int p, int q, int x, int y, int z, int w) {
	Chunk *chunk = find_chunk(p, q);
	if (chunk) {
		Map *map = &chunk->lights;
		if (map_set(map, x, y, z, w)) {
			dirty_chunk(chunk);
			db_insert_light(p, q, x, y, z, w);
		}
	}
	else {
		db_insert_light(p, q, x, y, z, w);
	}
}

void _set_block(int p, int q, int x, int y, int z, int w, int dirty) {
	Chunk *chunk = find_chunk(p, q);
	if (chunk) {
		Map *map = &chunk->map;
		if (map_set(map, x, y, z, w)) {
			if (dirty) {
				dirty_chunk(chunk);
			}
			db_insert_block(p, q, x, y, z, w);
		}
	}
	else {
		db_insert_block(p, q, x, y, z, w);
	}
	if (w == 0 && chunked(x) == p && chunked(z) == q) {
		unset_sign(x, y, z);
		set_light(p, q, x, y, z, 0);
	}
}

void set_block(int x, int y, int z, int w) {
	int p = chunked(x);
	int q = chunked(z);
	_set_block(p, q, x, y, z, w, 1);
	for (int dx = -1; dx <= 1; dx++) {
		for (int dz = -1; dz <= 1; dz++) {
			if (dx == 0 && dz == 0) {
				continue;
			}
			if (dx && chunked(x + dx) == p) {
				continue;
			}
			if (dz && chunked(z + dz) == q) {
				continue;
			}
			_set_block(p + dx, q + dz, x, y, z, -w, 1);
		}
	}
	client_block(x, y, z, w);
}

int player_intersects_block(
		int height,
		float x, float y, float z,
		int hx, int hy, int hz)
{
	int nx = roundf(x);
	int ny = roundf(y);
	int nz = roundf(z);
	for (int i = 0; i < height; i++) {
		if (nx == hx && ny - i == hy && nz == hz) {
			return 1;
		}
	}
	return 0;
}

Player *find_player(int id) {
	for (int i = 0; i < g->player_count; i++) {
		Player *player = g->players + i;
		if (player->id == id) {
			return player;
		}
	}
	return 0;
}

void update_player(Player *player,
				   float x, float y, float z, float rx, float ry, int interpolate)
{
	if (interpolate) {
		State *s1 = &player->state1;
		State *s2 = &player->state2;
		memcpy(s1, s2, sizeof(State));
		s2->x = x; s2->y = y; s2->z = z; s2->rx = rx; s2->ry = ry;
		s2->t = glfwGetTime();
		if (s2->rx - s1->rx > PI) {
			s1->rx += 2 * PI;
		}
		if (s1->rx - s2->rx > PI) {
			s1->rx -= 2 * PI;
		}
	}
	else {
		State *s = &player->state;
		s->x = x; s->y = y; s->z = z; s->rx = rx; s->ry = ry;
		del_buffer(player->buffer);
		player->buffer = gen_player_buffer(s->x, s->y, s->z, s->rx, s->ry);
	}
}

void delete_player(int id) {
	Player *player = find_player(id);
	if (!player) {
		return;
	}
	int count = g->player_count;
	del_buffer(player->buffer);
	Player *other = g->players + (--count);
	memcpy(player, other, sizeof(Player));
	g->player_count = count;
}
