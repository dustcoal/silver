#include "common/util/string_util.h"

char *strjoin(const char *str1, const char *str2) {
	if (!str1 && !str2) {
		return (strdup(""));
	} else if (!str1) {
		return strdup(str2);
	} else if (!str2) {
		return strdup(str1);
	}
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);
	size_t total_length = len1 + len2 + 1; // +1 for the null terminator

	char *res = malloc(total_length);

	if (!res) {
		return (NULL);
	}

	// Copy the first string to the result
	strcpy(res, str1);

	// Concatenate the second string to the result
	strcat(res, str2);

	return (res);
}
