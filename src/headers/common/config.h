#ifndef CRAFT_CONFIG_H
#define CRAFT_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cJSON.h>

#include "common/logging.h"
#include "common/util/pointer_util.h"
#include "common/util/min_max.h"

typedef cJSON jsonConfig;
typedef cJSON configSection;

/* generate new config with a path and all (it's a root config, not a section) */
jsonConfig  *new_conf(char *path, char *name, char *displayname, char *description, void *(*populate_func)(jsonConfig *));

/* adds a section to a section or config */
configSection *add_section(configSection *target_section, char *name, char *displayname, char *description);

/* returns the struct corresponding to the json. the `populate_func` should use
 * appropriate getters, so in case of errors such as missing values, the struct still is
 * populated with sensible values
 * */
void *populate_config_struct(jsonConfig *cfg);

#endif
