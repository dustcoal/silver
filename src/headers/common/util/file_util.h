#ifndef CRAFT_FILE_UTIL_H
#define CRAFT_FILE_UTIL_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

int file_readable(const char *file_path);
char *read_file(const char *file_path);
void writeFile(const char *file_path, const char *content);

#endif //CRAFT_FILE_UTIL_H
