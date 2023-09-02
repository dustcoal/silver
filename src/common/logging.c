#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common/logging.h"
#include "common/common.h"

/*#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
#include <VersionHelpers.h>
#elif __linux__
#include <sys/sysinfo.h>
#include <GLFW/glfw3.h>
#include <unistd.h>
#endif


#ifdef __linux__
// Function to detect the display server
const char* detectDisplayServer() {
	const char* displayServer = "Unknown";

	// Check for Wayland
	if (getenv("WAYLAND_DISPLAY")) {
		displayServer = "Wayland";
	} else if (getenv("DISPLAY")) {
		displayServer = "X11";
	}

	return displayServer;
}
#endif
*/

/* returns pointer to the beginning of the substring */
const char* get_filename(const char* path) {
	int i;
	const char *filename = path;

	// Find the last directory separator or backslash, depending on the OS
	for (i = strlen(path) - 1; i >= 0; i--) {
		if (path[i] == '/' || path[i] == '\\') {
			filename = &path[i + 1];
			break ;
		}
	}

	return filename;
}

void get_timestamp(char *timestamp) {
	time_t raw_time;
	struct tm *time_info;

	// Get the current time
	if(time(&raw_time) == -1) {
		for (int i = 0; i < 8; ++i) {
			timestamp[i] = -1;
		}
		timestamp[8] = '\0';
		return ;
	}
	time_info = localtime(&raw_time);
	if (time_info == NULL) {
		for (int i = 0; i < 8; ++i) {
			timestamp[i] = -1;
		}
		timestamp[8] = '\0';
		return ;
	}

	// Extract the time components and format them
	timestamp[0] = '0' + (time_info->tm_hour / 10);
	timestamp[1] = '0' + (time_info->tm_hour % 10);
	timestamp[2] = ':';
	timestamp[3] = '0' + (time_info->tm_min / 10);
	timestamp[4] = '0' + (time_info->tm_min % 10);
	timestamp[5] = ':';
	timestamp[6] = '0' + (time_info->tm_sec / 10);
	timestamp[7] = '0' + (time_info->tm_sec % 10);
	timestamp[8] = '\0';
}

int print_log(enum Enum_Log_Severity severity, const char *file, const char *func, char *content) {
	char *severity_symbol;
	char *side_symbol;
	char timestamp[9];
	const char* filename_without_extension;
	char file_func_concat[100];

	switch (severity) {
		case DEBUG:
			severity_symbol = "DEBUG";
			break;
		case INFO:
			severity_symbol = "INFO";
			break;
		case WARNING:
			severity_symbol = "WARN";
			break;
		case ERROR:
			severity_symbol = "ERROR";
			break;
		case FATAL:
			severity_symbol = "FATAL";
			break;
		default:
			severity_symbol = "UNKNOWN";
	}

	switch (NET_SIDE) {
		case CLIENT:
			side_symbol = "CLIENT";
			break;
		case SERVER:
			side_symbol = "SERVER";
			break;
		default:
			side_symbol = "UNKNOWN";
	}

	get_timestamp(timestamp);
	filename_without_extension = get_filename(file);
	snprintf(file_func_concat, sizeof(file_func_concat), "%s:%s", filename_without_extension, func);
	if (printf("[%s] [%s/%s] [%s]: %s\n", timestamp, side_symbol, severity_symbol, file_func_concat, content) < 0) {
		return (0);
	}
	return (1);
}

/*
void log_system_info() {
	printf("System info:\n");
	#ifdef _WIN32
		// Get CPU information
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		printf("CPU Cores: %u\n", sysInfo.dwNumberOfProcessors);

		// Get memory information
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(memInfo);
		GlobalMemoryStatusEx(&memInfo);
		printf("Total Physical Memory: %llu MB\n", memInfo.ullTotalPhys / (1024 * 1024));

		// Get GPU information
		// You may need to use a specific GPU API for more detailed information
		printf("GPU: To be implemented\n");

		// Get screen resolution
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		printf("Screen Resolution: %dx%d\n", screenWidth, screenHeight);

		// Get OS version
		if (IsWindows10OrGreater()) {
			printf("Operating System: Windows 10 or newer\n");
		} else {
			printf("Operating System: Older than Windows 10\n");
		}

		// Get GLFW version and backend
		printf("GLFW Version: %s\n", glfwGetVersionString());
		//printf("GLFW Backend: %s\n", glfwGetPlatformName());

	#elif __linux__
		// Get CPU information
		struct sysinfo info;
		sysinfo(&info);
		printf("CPU Cores: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));

		// Get memory information
		printf("Total Physical Memory: %ld MB\n", info.totalram / (1024 * 1024));

		// Get GPU information
		// You may need to use a specific GPU API for more detailed information
		printf("GPU: To be implemented\n");

		// Get screen resolution
		const char* displayServer = detectDisplayServer();
		printf("Display Server: %s\n", displayServer);

		// Get OS version
		printf("Operating System: Linux\n");

		// Get GLFW version and backend
		//int glfwInitialized = glfwInit();
		if (glfwInitialized) {
			int glfwMajor, glfwMinor, glfwRevision;
			glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
			printf("GLFW Version: %d.%d.%d\n", glfwMajor, glfwMinor, glfwRevision);
			const char* glfwBackend = glfwGetPlatformName();
			printf("GLFW Backend: %s\n", glfwBackend);
			glfwTerminate();
		} else {
			printf("GLFW not initialized\n");
		}
	#endif
}
 */
