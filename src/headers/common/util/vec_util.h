#ifndef CRAFT_VEC_UTIL_H
#define CRAFT_VEC_UTIL_H

/* system */
#include <string.h>

#include "vec.h"

void free_vec_str_allocated(vec_str_allocated vec);

char *vec_str_join(vec_str vec);
char *vec_str_join_sep(vec_str vec, const char *sep);

#endif //CRAFT_VEC_UTIL_H
