#include "logger.h"

LogLevel log_level = LOG_INFO;

void set_log_level(LogLevel level) { log_level = level; };

int log_message(LogLevel level, char *restrict format, ...) {
  if (level < log_level) {
    return 0;
  }

  char *level_str;

  switch (level) {
  case LOG_DEBUG:
    level_str = "DEBUG";
    break;
  case LOG_INFO_2:
    level_str = "INFO_2";
    break;
  case LOG_INFO:
    level_str = "INFO";
    break;
  case LOG_WARNING:
    level_str = "WARNING";
    break;
  case LOG_ERROR:
    level_str = "ERROR";
    break;
  default:
    level_str = "UNKNOWN";
    break;
  }

  printf("[%s] ", level_str);

  va_list args;
  va_start(args, format);
  int ret = vprintf(format, args);
  va_end(args);

  return ret;
}