#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3

#define MAX_LOG_MESSAGE 2048
#define MAX_LOG_FILE_PATH 512

typedef struct {
    char log_file[MAX_LOG_FILE_PATH];
    char name[64];
    int level;
    FILE *file_handle;
} Logger;

Logger* logger_init(const char *name, const char *log_file);
void logger_free(Logger *logger);
void logger_log(Logger *logger, int level, const char *format, ...);
void logger_info(Logger *logger, const char *format, ...);
void logger_error(Logger *logger, const char *format, ...);
void logger_warn(Logger *logger, const char *format, ...);
void logger_debug(Logger *logger, const char *format, ...);

#endif
