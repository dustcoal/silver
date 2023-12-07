/* system */
#include <stdlib.h>

/* deps */
#include "log.h"
#include "klist.h"

/* src/headers */
#include "common/common.h"
#include "common/util/windows_console_helper.h"

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
		log_debug("Debug mode enabled\n");
	}
}
