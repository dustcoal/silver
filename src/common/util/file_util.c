#include "common/util/file_util.h"

int file_readable(const char *file_path) {
	// Check if the file exists and is readable
	if (access(file_path, F_OK | R_OK) == 0) {
		return (1);
	} else {
		return (0);
	}
}

char *read_file(const char *file_path) {
	FILE *file = fopen(file_path, "rb"); // Open the file in binary mode

	if (file == NULL) {
		log_warn("Failed opening file: '%s'\n", file_path);
		perror("");
		return NULL;
	}

	// Determine the size of the file
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	// Allocate memory to store the file content
	char *buffer = (char *)malloc(fileSize + 1); // +1 for null terminator

	if (buffer == NULL) {
		log_fatal(FAILEDALLOC);
		fclose(file);
		return NULL;
	}

	// Read the file content into the buffer
	size_t bytesRead = fread(buffer, 1, fileSize, file);

	if (bytesRead != fileSize) {
		log_warn("Failed reading file: '%s'\n", file_path);
		perror("");
		free(buffer);
		fclose(file);
		return NULL;
	}

	// Null-terminate the buffer
	buffer[fileSize] = '\0';

	// Close the file
	fclose(file);

	return (buffer);
}

void writeFile(const char *file_path, const char *content) {
	FILE *file = fopen(file_path, "w"); // Open the file in write mode

	if (file == NULL) {
		log_warn("Failed opening file: '%s'\n", file_path);
		perror("");
		return;
	}

	// Write the content to the file
	if (fprintf(file, "%s", content) < 0) {
		log_warn("Failed writing file: '%s'\n", file_path);
		perror("");
	}

	// Close the file
	fclose(file);
}
