#include "client/network.h"

void parse_buffer(char *buffer) {
	Player *me = g->players;
	State *s = &g->players->state;
	char *key;
	char *line = tokenize(buffer, "\n", &key);
	while (line) {
		int pid;
		float ux, uy, uz, urx, ury;
		if (sscanf(line, "U,%d,%f,%f,%f,%f,%f",
				   &pid, &ux, &uy, &uz, &urx, &ury) == 6)
		{
			me->id = pid;
			s->x = ux; s->y = uy; s->z = uz; s->rx = urx; s->ry = ury;
			force_chunks(me);
			if (uy == 0) {
				s->y = highest_block(s->x, s->z) + 2;
			}
		}
		int bp, bq, bx, by, bz, bw;
		if (sscanf(line, "B,%d,%d,%d,%d,%d,%d",
				   &bp, &bq, &bx, &by, &bz, &bw) == 6)
		{
			_set_block(bp, bq, bx, by, bz, bw, 0);
			if (player_intersects_block(2, s->x, s->y, s->z, bx, by, bz)) {
				s->y = highest_block(s->x, s->z) + 2;
			}
		}
		if (sscanf(line, "L,%d,%d,%d,%d,%d,%d",
				   &bp, &bq, &bx, &by, &bz, &bw) == 6)
		{
			set_light(bp, bq, bx, by, bz, bw);
		}
		float px, py, pz, prx, pry;
		if (sscanf(line, "P,%d,%f,%f,%f,%f,%f",
				   &pid, &px, &py, &pz, &prx, &pry) == 6)
		{
			Player *player = find_player(pid);
			if (!player && g->player_count < MAX_PLAYERS) {
				player = g->players + g->player_count;
				g->player_count++;
				player->id = pid;
				player->buffer = 0;
				snprintf(player->name, MAX_NAME_LENGTH, "player%d", pid);
				update_player(player, px, py, pz, prx, pry, 1); // twice
			}
			if (player) {
				update_player(player, px, py, pz, prx, pry, 1);
			}
		}
		if (sscanf(line, "D,%d", &pid) == 1) {
			delete_player(pid);
		}
		int kp, kq, kk;
		if (sscanf(line, "K,%d,%d,%d", &kp, &kq, &kk) == 3) {
			db_set_key(kp, kq, kk);
		}
		if (sscanf(line, "R,%d,%d", &kp, &kq) == 2) {
			Chunk *chunk = find_chunk(kp, kq);
			if (chunk) {
				dirty_chunk(chunk);
			}
		}
		double elapsed;
		int day_length;
		if (sscanf(line, "E,%lf,%d", &elapsed, &day_length) == 2) {
			glfwSetTime(fmod(elapsed, day_length));
			g->day_length = day_length;
			g->time_changed = 1;
		}
		if (line[0] == 'T' && line[1] == ',') {
			char *text = line + 2;
			add_message(text);
		}
		char format[64];
		snprintf(
				format, sizeof(format), "N,%%d,%%%ds", MAX_NAME_LENGTH - 1);
		char name[MAX_NAME_LENGTH];
		if (sscanf(line, format, &pid, name) == 2) {
			Player *player = find_player(pid);
			if (player) {
				strncpy(player->name, name, MAX_NAME_LENGTH);
			}
		}
		snprintf(
				format, sizeof(format),
				"S,%%d,%%d,%%d,%%d,%%d,%%d,%%%d[^\n]", MAX_SIGN_LENGTH - 1);
		int face;
		char text[MAX_SIGN_LENGTH] = {0};
		if (sscanf(line, format,
				   &bp, &bq, &bx, &by, &bz, &face, text) >= 6)
		{
			_set_sign(bp, bq, bx, by, bz, face, text, 0);
		}
		line = tokenize(NULL, "\n", &key);
	}
}
