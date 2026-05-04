#include "monitoring.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <processthreadsapi.h>
#else
#include <sys/resource.h>
#include <unistd.h>
#endif

OperationMonitor* monitor_init(void) {
    OperationMonitor *mon = (OperationMonitor*)malloc(sizeof(OperationMonitor));
    if (!mon) return NULL;
    
    mon->logger = logger_init("OperationMonitor", NULL);
    mon->active = 0;
    mon->requests_sent = 0;
    mon->successful_requests = 0;
    mon->failed_requests = 0;
    mon->last_request_time = 0;
    mon->start_time = 0;
    mon->cpu_count = 0;
    mon->memory_count = 0;
    mon->active_threads = 0;
    mon->error_count = 0;
    
    memset(mon->cpu_usage, 0, sizeof(mon->cpu_usage));
    memset(mon->memory_usage, 0, sizeof(mon->memory_usage));
    
    return mon;
}

void monitor_free(OperationMonitor *mon) {
    if (mon) {
        logger_free(mon->logger);
        free(mon);
    }
}

void monitor_start(OperationMonitor *mon) {
    mon->active = 1;
    mon->start_time = time(NULL);
    logger_info(mon->logger, "Security testing monitoring started");
}

void monitor_stop(OperationMonitor *mon) {
    mon->active = 0;
    monitor_generate_report(mon);
}

void monitor_log_request(OperationMonitor *mon, int success) {
    mon->requests_sent++;
    mon->last_request_time = time(NULL);
    if (success) {
        mon->successful_requests++;
    } else {
        mon->failed_requests++;
    }
}

static double get_current_cpu_usage(void) {
#ifdef _WIN32
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return 5.0;
    }
    return 0.0;
#else
    return 0.0;
#endif
}

static double get_current_memory_usage(void) {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        if (GlobalMemoryStatusEx(&memStatus)) {
            return (double)pmc.WorkingSetSize / memStatus.ullTotalPhys * 100.0;
        }
    }
    return 0.0;
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return (double)usage.ru_maxrss / 1024.0;
    }
    return 0.0;
#endif
}

static int get_thread_count(void) {
#ifdef _WIN32
    return GetCurrentThreadId() ? 1 : 0;
#else
    return 1;
#endif
}

double monitor_get_cpu_usage(OperationMonitor *mon) {
    double usage = get_current_cpu_usage();
    if (mon->cpu_count < MAX_CPU_SAMPLES) {
        mon->cpu_usage[mon->cpu_count++] = usage;
    } else {
        for (int i = 1; i < MAX_CPU_SAMPLES; i++) {
            mon->cpu_usage[i - 1] = mon->cpu_usage[i];
        }
        mon->cpu_usage[MAX_CPU_SAMPLES - 1] = usage;
    }
    
    double sum = 0;
    int count = mon->cpu_count < 10 ? mon->cpu_count : 10;
    for (int i = mon->cpu_count - count; i < mon->cpu_count; i++) {
        if (i >= 0) sum += mon->cpu_usage[i];
    }
    return count > 0 ? sum / count : 0;
}

double monitor_get_memory_usage(OperationMonitor *mon) {
    double usage = get_current_memory_usage();
    if (mon->memory_count < MAX_MEMORY_SAMPLES) {
        mon->memory_usage[mon->memory_count++] = usage;
    } else {
        for (int i = 1; i < MAX_MEMORY_SAMPLES; i++) {
            mon->memory_usage[i - 1] = mon->memory_usage[i];
        }
        mon->memory_usage[MAX_MEMORY_SAMPLES - 1] = usage;
    }
    
    double sum = 0;
    int count = mon->memory_count < 10 ? mon->memory_count : 10;
    for (int i = mon->memory_count - count; i < mon->memory_count; i++) {
        if (i >= 0) sum += mon->memory_usage[i];
    }
    return count > 0 ? sum / count : 0;
}

void monitor_log_error(OperationMonitor *mon, const char *type, const char *details) {
    if (mon->error_count < MAX_ERROR_TYPES) {
        strncpy(mon->errors[mon->error_count].type, type, 63);
        mon->errors[mon->error_count].type[63] = '\0';
        strncpy(mon->errors[mon->error_count].details, details, MAX_ERROR_MSG - 1);
        mon->errors[mon->error_count].details[MAX_ERROR_MSG - 1] = '\0';
        mon->errors[mon->error_count].timestamp = time(NULL);
        mon->error_count++;
    }
    logger_error(mon->logger, "%s: %s", type, details);
}

void monitor_print_status(OperationMonitor *mon) {
    if (!mon->active || mon->start_time == 0) {
        printf("Monitoring not started\n");
        return;
    }
    
    time_t elapsed = time(NULL) - mon->start_time;
    double success_rate = mon->requests_sent > 0 ?
        (double)mon->successful_requests / mon->requests_sent * 100 : 0;
    
    printf("\n--- Current Status ---\n");
    printf("Running time: %ld seconds\n", elapsed);
    printf("Requests sent: %d\n", mon->requests_sent);
    printf("Success rate: %.2f%%\n", success_rate);
    printf("Active threads: %d\n", mon->active_threads);
    printf("Avg CPU: %.2f%%\n", monitor_get_cpu_usage(mon));
    printf("Avg Memory: %.2f%%\n", monitor_get_memory_usage(mon));
    printf("----------------------\n");
}

void monitor_generate_report(OperationMonitor *mon) {
    if (mon->start_time == 0) return;
    
    time_t elapsed = time(NULL) - mon->start_time;
    double success_rate = mon->requests_sent > 0 ?
        (double)mon->successful_requests / mon->requests_sent * 100 : 0;
    
    printf("\n========================================\n");
    printf("    SECURITY TESTING REPORT\n");
    printf("========================================\n");
    printf("Duration: %ld seconds\n", elapsed);
    printf("Total Tests: %d\n", mon->requests_sent);
    printf("Successful Tests: %d\n", mon->successful_requests);
    printf("Failed Tests: %d\n", mon->failed_requests);
    printf("Success Rate: %.2f%%\n", success_rate);
    printf("Avg CPU Usage: %.2f%%\n", monitor_get_cpu_usage(mon));
    printf("Avg Memory Usage: %.2f%%\n", monitor_get_memory_usage(mon));
    
    if (mon->error_count > 0) {
        printf("\nError Summary:\n");
        printf("--------------\n");
        for (int i = 0; i < mon->error_count; i++) {
            int count = 1;
            for (int j = 0; j < i; j++) {
                if (strcmp(mon->errors[i].type, mon->errors[j].type) == 0) {
                    count++;
                }
            }
            if (count == 1) {
                int total = 0;
                for (int j = 0; j < mon->error_count; j++) {
                    if (strcmp(mon->errors[i].type, mon->errors[j].type) == 0) {
                        total++;
                    }
                }
                printf("%s: %d occurrences\n", mon->errors[i].type, total);
            }
        }
    }
    
    printf("========================================\n");
    
    FILE *report = fopen("security_testing_report.txt", "w");
    if (report) {
        fprintf(report, "Security Testing Report\n");
        fprintf(report, "=======================\n\n");
        fprintf(report, "Duration: %ld seconds\n", elapsed);
        fprintf(report, "Total Tests: %d\n", mon->requests_sent);
        fprintf(report, "Successful: %d\n", mon->successful_requests);
        fprintf(report, "Failed: %d\n", mon->failed_requests);
        fprintf(report, "Success Rate: %.2f%%\n", success_rate);
        fclose(report);
    }
}
