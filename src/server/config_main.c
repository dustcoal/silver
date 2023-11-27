#include <cJSON.h>

#include "server/server.h"
#include "server/default_conf.h"
#include "common/logging.h"

/*
 * The config is a struct found in servrt/server.h. It is read and written into a json using cJSON.
 * */

/* This has to be better. serialized into json, certes, but wrapper functions need to take categories,
 * descriptions, min/max values, basically fancy cloth config in C, a bit more tedious to write (no fancy
 * java decorators), but should present nicely to the end user, allowing for a complete GUI config
 * */
//TODO: cloth config in C, server-client and vice versa config sync (e.g. modify server config from a privileged client)
char *create_default_config(void) {
	cJSON *json = cJSON_CreateObject();
	if (!json) {
		log_fatal(FAILEDALLOC, 1);
		return (NULL);
	}
	if(!cJSON_AddNumberToObject(json, "host", default_conf.host)) {
		log_fatal("ERROR", 1);
		return (NULL);
	}
	if(!cJSON_AddNumberToObject(json, "port", default_conf.port)) {
		log_fatal("ERROR", 1);
		return (NULL);
	}
	if(!cJSON_AddNumberToObject(json, "chunk_size", default_conf.chunk_size)) {
		log_fatal("ERROR", 1);
		return (NULL);
	}
}

int read_server_config(ServerConfig *config) {
	(void) config;

	return (0);
}
