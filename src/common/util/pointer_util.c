#include "common/util/pointer_util.h"

char *ptr_to_str(void *ptr) {
	size_t buf_size = sizeof(ptr);
	char *res = calloc(1, buf_size + 1);
	if (!res) {
		return (NULL);
	}
	memcpy(res, &ptr, buf_size);
	return (res);
}

void *str_to_ptr(char *str) {
	void *r;
	memcpy(&r, str, sizeof(str));
	return (r);
}
