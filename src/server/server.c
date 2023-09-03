#include <stdio.h>

#include "cJSON.h"

#include "world.h"
#include "common/common.h"
#include "common/logging.h"
#include "server/config_main.h"

//static const int CHUNK_SIZE = 32;
ServerConfig server_config;

int main(int argc, char *argv[]) {
	(void) argc;
	(void) argv;

	log_debug("debug log");
	log_warn("lol warning");
	log_error("lol eror");

	common_init(SERVER);
	logging_init();

	log_info("Server starting");
	if (!read_server_config(&server_config)) {
		log_fatal("Failed to parse config");
		return (1);
	}
	return (0);
}
