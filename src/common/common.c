#include <stdlib.h>

#include "common/common.h"
#include "common/logging.h"

int NET_SIDE = NONE;
int	DEBUG_MODE = 0;

int	common_init(enum Enum_Side side) {
	/* setting side */
	NET_SIDE = side;
	/* debug mode check */
	if (getenv("DEBUG")) {
		DEBUG_MODE = 1;
		log_debug("Debug mode enabled", 1);
	}
}
