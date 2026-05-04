#include "proxy_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

ProxyManager* proxy_manager_init(const char *proxy_file, const char *api_url) {
    (void)api_url;
    ProxyManager *mgr = (ProxyManager*)malloc(sizeof(ProxyManager));
    if (!mgr) return NULL;
    
    mgr->logger = logger_init("ProxyManager", NULL);
    mgr->proxy_count = 0;
    mgr->stats_count = 0;
    mgr->current_index = 0;
    mgr->min_rotation_interval = 30;
    mgr->timeout = 5;
    
    strncpy(mgr->test_url, "https://www.google.com", sizeof(mgr->test_url) - 1);
    mgr->test_url[sizeof(mgr->test_url) - 1] = '\0';
    
    if (proxy_file) {
        strncpy(mgr->proxy_file, proxy_file, sizeof(mgr->proxy_file) - 1);
        mgr->proxy_file[sizeof(mgr->proxy_file) - 1] = '\0';
    } else {
        mgr->proxy_file[0] = '\0';
    }
    
    srand((unsigned int)time(NULL));
    return mgr;
}

void proxy_manager_free(ProxyManager *mgr) {
    if (mgr) {
        logger_free(mgr->logger);
        free(mgr);
    }
}

static char* reformat_proxy(const char *proxy) {
    static char formatted[MAX_PROXY_LENGTH];
    if (strncmp(proxy, "http://", 7) != 0 && strncmp(proxy, "https://", 8) != 0) {
        snprintf(formatted, sizeof(formatted), "http://%s", proxy);
    } else {
        strncpy(formatted, proxy, sizeof(formatted) - 1);
        formatted[sizeof(formatted) - 1] = '\0';
    }
    return formatted;
}

void proxy_manager_load_from_file(ProxyManager *mgr) {
    if (!mgr || strlen(mgr->proxy_file) == 0) {
        logger_warn(mgr->logger, "No proxy file provided");
        return;
    }
    
    FILE *file = fopen(mgr->proxy_file, "r");
    if (!file) {
        logger_error(mgr->logger, "Proxy file not found: %s", mgr->proxy_file);
        return;
    }
    
    char line[MAX_PROXY_LENGTH];
    while (fgets(line, sizeof(line), file) && mgr->proxy_count < MAX_PROXIES) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        if (strlen(line) > 0) {
            char *formatted = reformat_proxy(line);
            strncpy(mgr->proxies[mgr->proxy_count], formatted, MAX_PROXY_LENGTH - 1);
            mgr->proxies[mgr->proxy_count][MAX_PROXY_LENGTH - 1] = '\0';
            mgr->proxy_count++;
        }
    }
    
    fclose(file);
    logger_info(mgr->logger, "Loaded %d proxies from %s", mgr->proxy_count, mgr->proxy_file);
}

char* proxy_manager_get_random(ProxyManager *mgr) {
    if (mgr->proxy_count == 0) {
        logger_warn(mgr->logger, "No proxies available");
        return NULL;
    }
    int idx = rand() % mgr->proxy_count;
    return mgr->proxies[idx];
}

char* proxy_manager_get_next(ProxyManager *mgr) {
    if (mgr->proxy_count == 0) {
        return NULL;
    }
    
    char *proxy = mgr->proxies[mgr->current_index];
    mgr->current_index = (mgr->current_index + 1) % mgr->proxy_count;
    
    return proxy;
}

void proxy_manager_update_stats(ProxyManager *mgr, const char *proxy, int success, double response_time) {
    for (int i = 0; i < mgr->stats_count; i++) {
        if (strcmp(mgr->stats[i].proxy, proxy) == 0) {
            if (success) {
                mgr->stats[i].success++;
                mgr->stats[i].response_time = response_time;
            } else {
                mgr->stats[i].failure++;
            }
            mgr->stats[i].last_used = time(NULL);
            return;
        }
    }
    
    if (mgr->stats_count < MAX_PROXY_STATS) {
        strncpy(mgr->stats[mgr->stats_count].proxy, proxy, MAX_PROXY_LENGTH - 1);
        mgr->stats[mgr->stats_count].proxy[MAX_PROXY_LENGTH - 1] = '\0';
        mgr->stats[mgr->stats_count].success = success ? 1 : 0;
        mgr->stats[mgr->stats_count].failure = success ? 0 : 1;
        mgr->stats[mgr->stats_count].response_time = response_time;
        mgr->stats[mgr->stats_count].last_used = time(NULL);
        mgr->stats_count++;
    }
}

int proxy_manager_validate(ProxyManager *mgr, const char *proxy) {
    RequestManager *req_mgr = request_manager_init(1.0, 3.0);
    if (!req_mgr) return 0;
    
    request_manager_set_proxy(req_mgr, proxy);
    
    HTTPRequest request = {0};
    strncpy(request.url, mgr->test_url, sizeof(request.url) - 1);
    request.header_count = 0;
    strcpy(request.data, "");
    
    time_t start = time(NULL);
    HTTPResponse *response = request_manager_send(req_mgr, &request);
    double response_time = difftime(time(NULL), start);
    
    int is_valid = 0;
    if (response) {
        is_valid = (response->status_code == 200);
        free(response);
    }
    
    proxy_manager_update_stats(mgr, proxy, is_valid, response_time);
    request_manager_free(req_mgr);
    
    return is_valid;
}

void proxy_manager_validate_all(ProxyManager *mgr) {
    logger_info(mgr->logger, "Validating proxies...");
    
    char valid_proxies[MAX_PROXIES][MAX_PROXY_LENGTH];
    int valid_count = 0;
    
    for (int i = 0; i < mgr->proxy_count; i++) {
        int is_valid = proxy_manager_validate(mgr, mgr->proxies[i]);
        if (is_valid) {
            strncpy(valid_proxies[valid_count], mgr->proxies[i], MAX_PROXY_LENGTH - 1);
            valid_proxies[valid_count][MAX_PROXY_LENGTH - 1] = '\0';
            valid_count++;
            logger_info(mgr->logger, "Proxy valid: %s", mgr->proxies[i]);
        } else {
            logger_warn(mgr->logger, "Proxy invalid: %s", mgr->proxies[i]);
        }
    }
    
    mgr->proxy_count = valid_count;
    for (int i = 0; i < valid_count; i++) {
        strncpy(mgr->proxies[i], valid_proxies[i], MAX_PROXY_LENGTH - 1);
        mgr->proxies[i][MAX_PROXY_LENGTH - 1] = '\0';
    }
    
    logger_info(mgr->logger, "%d valid proxies found", valid_count);
}

void proxy_manager_save_valid(ProxyManager *mgr, const char *file_path) {
    FILE *file = fopen(file_path, "w");
    if (!file) {
        logger_error(mgr->logger, "Failed to save proxies to %s", file_path);
        return;
    }
    
    for (int i = 0; i < mgr->proxy_count; i++) {
        fprintf(file, "%s\n", mgr->proxies[i]);
    }
    
    fclose(file);
    logger_info(mgr->logger, "Valid proxies saved to %s", file_path);
}

static int is_proxy_valid(ProxyManager *mgr, const char *proxy) {
    for (int i = 0; i < mgr->stats_count; i++) {
        if (strcmp(mgr->stats[i].proxy, proxy) == 0) {
            return mgr->stats[i].failure < mgr->stats[i].success * 3;
        }
    }
    return 1;
}

char* proxy_manager_get_best(ProxyManager *mgr) {
    char *best_proxy = NULL;
    double best_score = 999999.0;
    
    for (int i = 0; i < mgr->proxy_count; i++) {
        if (!is_proxy_valid(mgr, mgr->proxies[i])) continue;
        
        double failure_rate = 0;
        double response_time = 999.0;
        
        for (int j = 0; j < mgr->stats_count; j++) {
            if (strcmp(mgr->stats[j].proxy, mgr->proxies[i]) == 0) {
                int total = mgr->stats[j].success + mgr->stats[j].failure;
                if (total > 0) {
                    failure_rate = (double)mgr->stats[j].failure / total;
                }
                response_time = mgr->stats[j].response_time;
                break;
            }
        }
        
        double score = failure_rate * 10 + response_time;
        if (score < best_score) {
            best_score = score;
            best_proxy = mgr->proxies[i];
        }
    }
    
    return best_proxy;
}
