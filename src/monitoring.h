#ifndef MONITORING_H
#define MONITORING_H

#include "logging.h"
#include <time.h>

#define MAX_CPU_SAMPLES 100
#define MAX_MEMORY_SAMPLES 100
#define MAX_ERROR_TYPES 50
#define MAX_ERROR_MSG 256

typedef struct {
    char type[64];
    char details[MAX_ERROR_MSG];
    time_t timestamp;
} ErrorEntry;

typedef struct {
    Logger *logger;
    int active;
    int requests_sent;
    int successful_requests;
    int failed_requests;
    time_t last_request_time;
    time_t start_time;
    double cpu_usage[MAX_CPU_SAMPLES];
    int cpu_count;
    double memory_usage[MAX_MEMORY_SAMPLES];
    int memory_count;
    int active_threads;
    ErrorEntry errors[MAX_ERROR_TYPES];
    int error_count;
} OperationMonitor;

OperationMonitor* monitor_init(void);
void monitor_free(OperationMonitor *mon);
void monitor_start(OperationMonitor *mon);
void monitor_stop(OperationMonitor *mon);
void monitor_log_request(OperationMonitor *mon, int success);
double monitor_get_cpu_usage(OperationMonitor *mon);
double monitor_get_memory_usage(OperationMonitor *mon);
void monitor_log_error(OperationMonitor *mon, const char *type, const char *details);
void monitor_generate_report(OperationMonitor *mon);
void monitor_print_status(OperationMonitor *mon);

#endif
