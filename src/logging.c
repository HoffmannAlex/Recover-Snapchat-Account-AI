#include "logging.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#endif

static const char* level_names[] = {"DEBUG", "INFO", "WARN", "ERROR"};

static void ensure_directory_exists(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0755);
    }
}

Logger* logger_init(const char *name, const char *log_file) {
    Logger *logger = (Logger*)malloc(sizeof(Logger));
    if (!logger) return NULL;
    
    ensure_directory_exists("logs");
    
    strncpy(logger->name, name ? name : "default", sizeof(logger->name) - 1);
    logger->name[sizeof(logger->name) - 1] = '\0';
    
    if (log_file) {
        strncpy(logger->log_file, log_file, sizeof(logger->log_file) - 1);
        logger->log_file[sizeof(logger->log_file) - 1] = '\0';
    } else {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        snprintf(logger->log_file, sizeof(logger->log_file),
                 "logs/security_test_%04d%02d%02d_%02d%02d%02d.log",
                 tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
                 tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    }
    
    logger->level = LOG_LEVEL_INFO;
    logger->file_handle = fopen(logger->log_file, "a");
    
    return logger;
}

void logger_free(Logger *logger) {
    if (logger) {
        if (logger->file_handle) {
            fclose(logger->file_handle);
        }
        free(logger);
    }
}

void logger_log(Logger *logger, int level, const char *format, ...) {
    if (!logger || level < logger->level) return;
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    char message[MAX_LOG_MESSAGE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    const char *level_name = (level >= 0 && level <= 3) ? level_names[level] : "UNKNOWN";
    
    printf("%s - %s - %s - %s\n", timestamp, logger->name, level_name, message);
    
    if (logger->file_handle) {
        fprintf(logger->file_handle, "%s - %s - %s - %s\n", timestamp, logger->name, level_name, message);
        fflush(logger->file_handle);
    }
}

void logger_info(Logger *logger, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char message[MAX_LOG_MESSAGE];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    logger_log(logger, LOG_LEVEL_INFO, "%s", message);
}

void logger_error(Logger *logger, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char message[MAX_LOG_MESSAGE];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    logger_log(logger, LOG_LEVEL_ERROR, "%s", message);
}

void logger_warn(Logger *logger, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char message[MAX_LOG_MESSAGE];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    logger_log(logger, LOG_LEVEL_WARN, "%s", message);
}

void logger_debug(Logger *logger, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char message[MAX_LOG_MESSAGE];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    logger_log(logger, LOG_LEVEL_DEBUG, "%s", message);
}
