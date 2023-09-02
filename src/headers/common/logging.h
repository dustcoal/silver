#ifndef CRAFT_LOGGING_H
#define CRAFT_LOGGING_H

enum Enum_Log_Severity {
	DEBUG,
	INFO,
	WARNING,
	ERROR,
	FATAL
};

int print_log(enum Enum_Log_Severity severity, const char *file, const char *func, char *content);
#define log_debug(content) print_log(DEBUG, __FILE__, __func__, content)
#define log_info(content) print_log(INFO, __FILE__, __func__, content)
#define log_warn(content) print_log(WARNING, __FILE__, __func__, content)
#define log_error(content) print_log(ERROR, __FILE__, __func__, content)
#define log_fatal(content) print_log(FATAL, __FILE__, __func__, content)


#endif
