#include "common/util/vec_util.h"

void free_vec_str_allocated(vec_str_allocated vec) {
	for (int i = 0; i < vector_size(vec); ++i) {
		free(vec[i]);
	}
	vector_free(vec);
}

char *vec_str_join(vec_str vec) {
	size_t res_size = 0;
	char *res;

	if (!vec) {
		return (NULL);
	}
	size_t vec_size = vector_size(vec);
	for (int i = 0; i < vec_size; ++i) {
		res_size += strlen(vec[i]);
	}
	res = malloc(res_size + 1);
	if (!res) {
		return (NULL);
	}
	size_t index = 0;
	for (int i = 0; i < vec_size; ++i) {
		size_t len = strlen(vec[i]);
		memcpy(res + index, vec[i], len);
		index += len;
	}
	res[res_size] = '\0';
	return (res);
}

char *vec_str_join_sep(vec_str vec, const char *sep) {
	size_t res_size = 0;
	char *res;

	if (!vec || !sep) {
		return (NULL);
	}
	size_t vec_size = vector_size(vec);
	size_t sep_len = strlen(sep);
	size_t sep_total_len = (vec_size - 1) * sep_len;
	if (vec_size == 0) {
		sep_total_len = 0;
	}
	for (int i = 0; i < vec_size; ++i) {
		res_size += strlen(vec[i]);
	}
	res = malloc(res_size + sep_total_len + 1);
	if (!res) {
		return (NULL);
	}
	size_t index = 0;
	for (int i = 0; i < vec_size; ++i) {
		size_t len = strlen(vec[i]);
		memcpy(res + index, vec[i], len);
		index += len;
		if (i < vec_size - 1) {
			memcpy(res + index, sep, sep_len);
			index += sep_len;
		}
	}
	res[res_size + sep_total_len] = '\0';
	return (res);
}
