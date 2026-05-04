#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

#include <curl/curl.h>
#include <time.h>
#include "logging.h"

#define MAX_URL_LENGTH 2048
#define MAX_HEADERS 32
#define MAX_HEADER_LENGTH 512
#define MAX_RESPONSE_SIZE 65536
#define MAX_REQUESTS_HISTORY 100

typedef struct {
    time_t timestamp;
    int count;
} RequestRecord;

typedef struct {
    RequestRecord records[MAX_REQUESTS_HISTORY];
    int count;
    int max_requests;
    int time_window;
} RateLimiter;

typedef struct {
    char url[MAX_URL_LENGTH];
    char headers[MAX_HEADERS][MAX_HEADER_LENGTH];
    int header_count;
    char data[4096];
    char proxy[256];
    int use_proxy;
} HTTPRequest;

typedef struct {
    char body[MAX_RESPONSE_SIZE];
    size_t size;
    long status_code;
    char content_type[128];
} HTTPResponse;

typedef struct {
    Logger *logger;
    double min_delay;
    double max_delay;
    time_t last_request_time;
    RateLimiter limiter;
    char current_proxy[256];
    int use_proxy;
    CURL *curl;
} RequestManager;

typedef size_t (*WriteCallback)(void *contents, size_t size, size_t nmemb, void *userp);

RequestManager* request_manager_init(double min_delay, double max_delay);
void request_manager_free(RequestManager *mgr);
void request_manager_set_proxy(RequestManager *mgr, const char *proxy);
HTTPResponse* request_manager_send(RequestManager *mgr, HTTPRequest *request);
int request_manager_handle_rate_limit(RequestManager *mgr, HTTPResponse *response);
void request_manager_enforce_delay(RequestManager *mgr);
void rate_limiter_init(RateLimiter *limiter, int max_requests, int time_window);
int rate_limiter_allow(RateLimiter *limiter);
double rate_limiter_get_delay(RateLimiter *limiter);

#endif
