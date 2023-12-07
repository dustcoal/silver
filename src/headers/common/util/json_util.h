#ifndef CRAFT_JSON_UTIL_H
#define CRAFT_JSON_UTIL_H

#include <stdlib.h>

#include "cJSON.h"
#include "vec.h"


int *json_ArrayToArray(cJSON *arr);
vec_int jsonTo_vec_int(cJSON *arr);

#endif //CRAFT_JSON_UTIL_H
