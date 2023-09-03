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

int print_log(enum Enum_Log_Severity severity, const char *file, const char *func, char *content);
#define log_debug(content) print_log(DEBUG_LOG_SEVERITY, __FILE__, __func__, content)
#define log_info(content) print_log(INFO_LOG_SEVERITY, __FILE__, __func__, content)
#define log_warn(content) print_log(WARNING_LOG_SEVERITY, __FILE__, __func__, content)
#define log_error(content) print_log(ERROR_LOG_SEVERITY, __FILE__, __func__, content)
#define log_fatal(content) print_log(FATAL_LOG_SEVERITY, __FILE__, __func__, content)

int	unix_term_has_color();

#endif
