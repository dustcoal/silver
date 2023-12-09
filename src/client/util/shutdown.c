#include "client/util/shutdown.h"

void _shutdown(int code, int level, const char *file, int line, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_log(level, file, line, fmt, args);
	va_end(args);

	printf("Client shutdown is a stub!\n");
}
