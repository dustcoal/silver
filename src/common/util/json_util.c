#include "common/util/json_util.h"

int *json_ArrayToArray(cJSON *arr) {
	int s = cJSON_GetArraySize(arr);
	int *res = malloc(s * sizeof(*res));
	if (!res) {
		return (NULL);
	}
	for (int i = 0; i < s; ++i) {
		res[i] = cJSON_GetArrayItem(arr, i)->valueint;
	}
	return (res);
}

vec_int jsonTo_vec_int(cJSON *arr) {
	vec_int res = vector_create();
	if (!res) {
		return (NULL);
	}
	int s = cJSON_GetArraySize(arr);
	for (int i = 0; i < s; ++i) {
		if (!vector_add(&res, cJSON_GetArrayItem(arr, i)->valueint)) {
			vector_free(res);
			return (NULL);
		}
	}
	return (res);
}
