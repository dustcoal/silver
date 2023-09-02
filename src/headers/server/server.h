#ifndef CRAFT_SERVER_H
#define CRAFT_SERVER_H

typedef struct {
	int host;
	int port;
	int chunk_size;
	char **allowed_users; /* just for testing */
} ServerConfig;

#endif
