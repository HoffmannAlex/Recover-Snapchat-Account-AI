#ifndef CSRF_MANAGER_H
#define CSRF_MANAGER_H

#include "logging.h"
#include "request_manager.h"
#include <time.h>

#define CSRF_CACHE_DURATION 900
#define MAX_CSRF_TOKEN 512
#define MAX_CSRF_CACHE 10

typedef struct {
    char token[MAX_CSRF_TOKEN];
    time_t timestamp;
} CachedToken;

typedef struct {
    Logger *logger;
    char login_url[MAX_URL_LENGTH];
    int timeout;
    int cache_duration;
    CachedToken cache[MAX_CSRF_CACHE];
    int cache_count;
} CSRFManager;

CSRFManager* csrf_manager_init(const char *login_url, int timeout);
void csrf_manager_free(CSRFManager *mgr);
char* csrf_manager_get_token(CSRFManager *mgr, int use_proxy, const char *proxy);
char* csrf_manager_get_cached(CSRFManager *mgr);
void csrf_manager_cache_token(CSRFManager *mgr, const char *token);
char* csrf_manager_refresh(CSRFManager *mgr, int use_proxy, const char *proxy);
int csrf_manager_validate(CSRFManager *mgr, const char *token, int use_proxy, const char *proxy);
char* csrf_manager_extract_from_html(const char *html);

#endif
