/* system */
#include <stdlib.h>

/* deps */
#include "log.h"
#include "klist.h"

/* src/headers */
#include "common/common.h"
#include "common/util/windows_console_helper.h"
#include "define_config.h"

int NET_SIDE = NONE;
int	DEBUG_MODE = 0;

int	common_init(enum Enum_Side side) {
	/* setting side */
	NET_SIDE = side;
	/* Enable windows console color support */
	windows_console_enable_colors();

	/* debug mode check */
	if (getenv("DEBUG")) {
		DEBUG_MODE = 1;
	} else {
		DEBUG_MODE = DEBUG;
	}

	/* Init logging */
	logging_init(stdout, DEBUG_MODE);

	if (DEBUG_MODE) {
		log_debug("Debug mode enabled\n");
	}
}
