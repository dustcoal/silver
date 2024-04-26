/*
 * Copyright (c) 2020 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include "log.h"

#define MAX_CALLBACKS 32

#define BLACK "\x001b[30m"
#define B_BLACK "\x001b[40m"
#define RED "\x001b[31m"
#define B_RED "\x001b[41m"
#define GREEN "\x001b[30m"
#define B_GREEN "\x001b[40m"
#define YELLOW "\x001b[33m"
#define B_YELLLOW "\x001b[43m"
#define BLUE "\x001b[30m"
#define B_BLUE "\x001b[40m"
#define MAGENTA "\x001b[30m"
#define B_MAGENTA "\x001b[40m"
#define CYAN "\x001b[30m"
#define B_CYAN "\x001b[40m"
#define WHITE "\x001b[30m"
#define B_WHITE "\x001b[40m"
#define DEFAULT "\x001b[30m"
#define B_DEFAULT "\x001b[40m"
#define BRIGHT_BLACK "\x001b[90m"
#define B_BRIGHT_BLACK "\x001b[100m"
#define BRIGHT_RED "\x001b[91m"
#define B_BRIGHT_RED "\x001b[101m"
#define NC "\x001b[0m"


int	term_has_color = 0;
FILE *console_file = NULL; //stderr or stdout
int logging_debug = 0;

typedef struct {
  log_LogFn fn;
  void *udata;
  int level;
} Callback;

static struct {
  void *udata;
  log_LockFn lock;
  int level;
  bool quiet;
  Callback callbacks[MAX_CALLBACKS];
} L;

int logging_init(FILE *console_file_, int is_debug) {
#ifdef __linux__
	term_has_color = unix_term_has_color();
#endif
	console_file = console_file_;
	logging_debug = is_debug;
	return (1);
}

/* checks if unix terminal can display color */
#ifdef UNIX
int	unix_term_has_color() {
	#ifdef  WINDOWS
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
		log_debug("Tput not available", 1);
		return (0);
	}
	if (system("tput setaf 1 > /dev/null 2>&1") == 0) {
		return (1);
	} else {
		log_debug("Tput failed, terminal has no color", 1);
		return(0);
	}
}
#endif

/*
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
 * */


static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "", "", "", YELLOW, RED, B_BLACK BRIGHT_RED
};
#endif


static void stdout_callback(log_Event *ev) {
  char buf[16];
  buf[strftime(buf, sizeof(buf), "[%H:%M:%S]", &ev->time)] = '\0';
#ifdef LOG_USE_COLOR

  fprintf(
    ev->udata, "%s [%s/%s%s"NC"] "BRIGHT_BLACK"%s:%d:\x1b[0m ",
    buf, NET_SIDE_SYMBOL, level_colors[ev->level], level_strings[ev->level],
    ev->file, ev->line);
#else
  fprintf(
    ev->udata, "%s %-5s %s:%d: ",
    buf, level_strings[ev->level], ev->file, ev->line);
#endif
  vfprintf(ev->udata, ev->fmt, ev->ap);
  fprintf(ev->udata, "\n");
  fflush(ev->udata);
}


static void file_callback(log_Event *ev) {
  char buf[64];
  buf[strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", &ev->time)] = '\0';
  fprintf(
    ev->udata, "%s [%s/%s] %s:%d: ",
    buf, NET_SIDE_SYMBOL, level_strings[ev->level], ev->file, ev->line);
  vfprintf(ev->udata, ev->fmt, ev->ap);
  fprintf(ev->udata, "\n");
  fflush(ev->udata);
}


static void lock(void)   {
  if (L.lock) { L.lock(true, L.udata); }
}


static void unlock(void) {
  if (L.lock) { L.lock(false, L.udata); }
}


const char* log_level_string(int level) {
  return level_strings[level];
}


void log_set_lock(log_LockFn fn, void *udata) {
  L.lock = fn;
  L.udata = udata;
}


void log_set_level(int level) {
  L.level = level;
}


void log_set_quiet(bool enable) {
  L.quiet = enable;
}


int log_add_callback(log_LogFn fn, void *udata, int level) {
  for (int i = 0; i < MAX_CALLBACKS; i++) {
    if (!L.callbacks[i].fn) {
      L.callbacks[i] = (Callback) { fn, udata, level };
      return 0;
    }
  }
  return -1;
}


int log_add_fp(FILE *fp, int level) {
  return log_add_callback(file_callback, fp, level);
}

static void init_event(log_Event *ev, void *udata) {
  if (!ev->time_initted) {
    ev->time_initted = 1;
    time_t t = time(NULL);
    #ifdef WINDOWS
        localtime_s(&ev->time, &t);
    #else
        localtime_r(&t, &ev->time); // http://jianewyork.blogspot.com/2018/03/use-of-localtime-localtimer-and-their.html
    #endif
    cwk_path_get_basename(ev->file, &ev->file, NULL);
  }
  ev->udata = udata;
}


void log_log(int level, const char *file, int line, const char *fmt, ...) {
	if (level == LOG_DEBUG && !logging_debug) {
		return ;
	}
    log_Event ev = {
        .fmt   = fmt,
        .file  = file,
        .line  = line,
        .level = level,
        .time_initted = 0
    };

    lock();

    if (!L.quiet && level >= L.level) {
        init_event(&ev, console_file);
        va_start(ev.ap, fmt);
        stdout_callback(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback *cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    unlock();
}
