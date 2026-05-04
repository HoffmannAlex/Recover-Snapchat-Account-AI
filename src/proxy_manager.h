#ifndef PROXY_MANAGER_H
#define PROXY_MANAGER_H

#include "logging.h"
#include "request_manager.h"

#define MAX_PROXIES 1000
#define MAX_PROXY_LENGTH 256
#define MAX_PROXY_STATS 1000

typedef struct {
    char proxy[MAX_PROXY_LENGTH];
    int success;
    int failure;
    time_t last_used;
    double response_time;
} ProxyStats;

typedef struct {
    Logger *logger;
    char proxies[MAX_PROXIES][MAX_PROXY_LENGTH];
    int proxy_count;
    char proxy_file[MAX_PROXY_LENGTH];
    char test_url[MAX_PROXY_LENGTH];
    int timeout;
    ProxyStats stats[MAX_PROXY_STATS];
    int stats_count;
    int min_rotation_interval;
    int current_index;
} ProxyManager;

ProxyManager* proxy_manager_init(const char *proxy_file, const char *api_url);
void proxy_manager_free(ProxyManager *mgr);
void proxy_manager_load_from_file(ProxyManager *mgr);
void proxy_manager_fetch_from_api(ProxyManager *mgr, const char *api_url);
char* proxy_manager_get_next(ProxyManager *mgr);
char* proxy_manager_get_random(ProxyManager *mgr);
void proxy_manager_update_stats(ProxyManager *mgr, const char *proxy, int success, double response_time);
int proxy_manager_validate(ProxyManager *mgr, const char *proxy);
void proxy_manager_validate_all(ProxyManager *mgr);
void proxy_manager_save_valid(ProxyManager *mgr, const char *file_path);
char* proxy_manager_get_best(ProxyManager *mgr);

#endif
