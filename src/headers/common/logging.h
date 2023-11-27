#ifndef CRAFT_LOGGING_H
#define CRAFT_LOGGING_H

int logging_init();

enum Enum_Log_Severity {
	DEBUG_LOG_SEVERITY,
	INFO_LOG_SEVERITY,
	WARNING_LOG_SEVERITY,
	ERROR_LOG_SEVERITY,
	FATAL_LOG_SEVERITY
};

#define FAILEDALLOC "Failed Allocation\n"

int print_log(enum Enum_Log_Severity severity, const char *file, const char *func, char *content, int newline);
#define log_debug(content, newline) print_log(DEBUG_LOG_SEVERITY, __FILE__, __func__, content, newline)
#define log_info(content, newline) print_log(INFO_LOG_SEVERITY, __FILE__, __func__, content, newline)
#define log_warn(content, newline) print_log(WARNING_LOG_SEVERITY, __FILE__, __func__, content, newline)
#define log_error(content, newline) print_log(ERROR_LOG_SEVERITY, __FILE__, __func__, content, newline)
#define log_fatal(content, newline) print_log(FATAL_LOG_SEVERITY, __FILE__, __func__, content, newline)

int	unix_term_has_color();

#endif
