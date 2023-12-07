#ifndef CRAFT_CONFIG_H
#define CRAFT_CONFIG_H

/* system */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* deps */
#include "cJSON.h"
#include "log.h"
#include "vec.h"

/* own */
#include "common/util/pointer_util.h"
#include "common/util/min_max.h"

typedef cJSON jsonConfig;
typedef cJSON configSection;

/* generate new config with a path and all (it's a root config, not a section) */
jsonConfig  *new_conf(char *path, char *name, char *displayname, char *description, void *(*populate_func)(jsonConfig *));

/* gets/adds a section to a section or config */
configSection *get_section(configSection *target_section, char *name, char *displayname, char *description);

void save_conf(jsonConfig *cfg);

/* returns the struct corresponding to the json. the `populate_func` should use
 * appropriate getters, so in case of errors such as missing values, the struct still is
 * populated with sensible values
 * */
void *populate_config_struct(jsonConfig *cfg);

/* functions getting and setting config values */
int get_int_val(jsonConfig *cfg, char *name, char *comment, int default_val, int min_val, int max_val);
float get_float_val(jsonConfig *cfg, char *name, char *comment, float default_val, float min_val, float max_val);
int get_bool_val(jsonConfig *cfg, char *name, char *comment, int default_val);
char *get_string_val(jsonConfig *cfg, char *name, char *comment, const char *default_val);
vec_int get_int_vec(jsonConfig *cfg, char *name, char *comment, int *default_val);

#endif
