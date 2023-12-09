#ifndef CRAFT_SHUTDOWN_H
#define CRAFT_SHUTDOWN_H

#include "log.h"

#include "common/common.h"

#define shutdown( code, ...) _shutdown(code, LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
void _shutdown(int code, int level, const char *file, int line, const char *fmt, ...);

#endif //CRAFT_SHUTDOWN_H
