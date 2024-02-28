#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdarg.h>
#include <stdio.h>

typedef enum {
  LOG_DEBUG,
  LOG_INFO_2,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR
} LogLevel;
void set_log_level(LogLevel level);
int log_message(LogLevel level, char *restrict format, ...);
int log_message_line(LogLevel level, int linenum, char *restrict format, ...);

#endif