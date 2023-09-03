#ifdef NO_OLDNAMES
#undef NO_OLDNAMES
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux__
#include <unistd.h>
#endif

#include "common/logging.h"
#include "common/common.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#define YELLOW "\x001b[33m"
#define RED "\x001b[91m"
#define DARKRED "\x001b[31m"
#define NC "\x001b[0m"
#define BACKGROUND_BLACK "\x001b[40m"
#elif _WIN32
#define FOREGROUND_WHITE	(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_YELLOW	(FOREGROUND_RED | FOREGROUND_GREEN)
#define FOREGROUND_INTENSE_YELLOW	(FOREGROUND_YELLOW | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_RED	(FOREGROUND_RED | FOREGROUND_INTENSITY)
#define BACKGROUND_INTENSE_WHITE	(BACKGROUND_WHITE | BACKGROUND_INTENSITY)
#define BACKGROUND_CYAN	(BACKGROUND_BLUE | BACKGROUND_GREEN)
#endif

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

int	term_has_color = 0;


int logging_init() {
	#ifdef __linux__
	term_has_color = unix_term_has_color();
	#endif
}

/* checks if unix terminal can display color */
#ifdef __linux__
int	unix_term_has_color() {
	#ifdef  _WIN32
	return (0);
	#endif
	const char *command = "which tput";
	int tput_available;

	// Use the popen function to run the "which tput" command and capture its output
	FILE *fp = popen(command, "r");
	if (fp == NULL) {
		perror("popen");
		return 1;
	}
	// Read the command output
	char output[1024];
	if (fgets(output, sizeof(output), fp) != NULL) {
		tput_available = 1;
	} else {
		// If "tput" was not found, it's not available
		tput_available = 0;
	}
	// Close the file pointer
	pclose(fp);
	if (!tput_available) {
		log_debug("Tput not available");
		return (0);
	}
	if (system("tput setaf 1 > /dev/null 2>&1") == 0) {
		return (1);
	} else {
		log_debug("Tput failed, terminal has no color");
		return(0);
	}
}
#endif

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

	if (severity == DEBUG_LOG_SEVERITY && !DEBUG_MODE) {
		return (1);
	}

	switch (severity) {
		case DEBUG_LOG_SEVERITY:
			severity_symbol = "DEBUG";
			break;
		case INFO_LOG_SEVERITY:
			severity_symbol = "INFO";
			break;
		case WARNING_LOG_SEVERITY:
			#ifdef __linux__
			if (term_has_color)
				severity_symbol = YELLOW"WARN"NC;
			else
				severity_symbol = "WARN";
			#elif _WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSE_YELLOW);
			severity_symbol = "WARN";
			#else
			severity_symbol = "WARN";
			#endif
			break;
		case ERROR_LOG_SEVERITY:
			#ifdef __linux__
			if (term_has_color)
				severity_symbol = RED"ERROR"NC;
			else
				severity_symbol = "ERROR";
			#elif _WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
			severity_symbol = "ERROR";
			#else
			severity_symbol = "ERROR";
			#endif
			break;
		case FATAL_LOG_SEVERITY:
			#ifdef __linux__
			if (term_has_color)
				severity_symbol = BACKGROUND_BLACK DARKRED"FATAL"NC;
			else
				severity_symbol = "FATAL";
			#elif _WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSE_RED | BACKGROUND_BLUE);
			severity_symbol = "FATAL";
			#else
			severity_symbol = "FATAL";
			#endif
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
	#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_WHITE);
	#endif
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
