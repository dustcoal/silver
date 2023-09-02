#!/bin/sh

gcc -std=c99 -O3 -Wall -Wextra -Werror -pedantic -o server -I src -I src/headers -I deps/noise deps/noise/noise.c -I deps/cJSON src/world.c src/server/server.c src/server/config_main.c src/common/common.c src/common/logging.c
