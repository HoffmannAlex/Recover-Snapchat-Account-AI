#include "csrf_manager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

CSRFManager* csrf_manager_init(const char *login_url, int timeout) {
    CSRFManager *mgr = (CSRFManager*)malloc(sizeof(CSRFManager));
    if (!mgr) return NULL;
    
    mgr->logger = logger_init("CSRFManager", NULL);
    mgr->timeout = timeout;
    mgr->cache_duration = CSRF_CACHE_DURATION;
    mgr->cache_count = 0;
    
    if (login_url) {
        strncpy(mgr->login_url, login_url, MAX_URL_LENGTH - 1);
    } else {
        strncpy(mgr->login_url, "https://www.snapchat.com/accounts/login/", MAX_URL_LENGTH - 1);
    }
    mgr->login_url[MAX_URL_LENGTH - 1] = '\0';
    
    return mgr;
}

void csrf_manager_free(CSRFManager *mgr) {
    if (mgr) {
        logger_free(mgr->logger);
        free(mgr);
    }
}

char* csrf_manager_get_cached(CSRFManager *mgr) {
    time_t now = time(NULL);
    for (int i = 0; i < mgr->cache_count; i++) {
        if (now - mgr->cache[i].timestamp < mgr->cache_duration) {
            logger_info(mgr->logger, "Returning cached CSRF token");
            return mgr->cache[i].token;
        }
    }
    return NULL;
}

void csrf_manager_cache_token(CSRFManager *mgr, const char *token) {
    if (mgr->cache_count < MAX_CSRF_CACHE) {
        strncpy(mgr->cache[mgr->cache_count].token, token, MAX_CSRF_TOKEN - 1);
        mgr->cache[mgr->cache_count].token[MAX_CSRF_TOKEN - 1] = '\0';
        mgr->cache[mgr->cache_count].timestamp = time(NULL);
        mgr->cache_count++;
        logger_info(mgr->logger, "CSRF token cached");
    }
}

char* csrf_manager_extract_from_html(const char *html) {
    static char token[MAX_CSRF_TOKEN];
    const char *pattern = "\"csrf_token\":\"";
    const char *start = strstr(html, pattern);
    
    if (!start) {
        pattern = "csrf_token";
        start = strstr(html, pattern);
        if (!start) return NULL;
        
        start += strlen(pattern);
        while (*start && (*start == '"' || *start == ':' || *start == ' ' || *start == '=')) {
            start++;
        }
        if (*start == '"') start++;
    } else {
        start += strlen(pattern);
    }
    
    int i = 0;
    while (*start && *start != '"' && i < MAX_CSRF_TOKEN - 1) {
        token[i++] = *start++;
    }
    token[i] = '\0';
    
    return (i > 0) ? token : NULL;
}

char* csrf_manager_get_token(CSRFManager *mgr, int use_proxy, const char *proxy) {
    char *cached = csrf_manager_get_cached(mgr);
    if (cached) {
        return cached;
    }
    
    RequestManager *req_mgr = request_manager_init(1.0, 3.0);
    if (!req_mgr) return NULL;
    
    if (use_proxy && proxy) {
        request_manager_set_proxy(req_mgr, proxy);
    }
    
    HTTPRequest request = {0};
    strncpy(request.url, mgr->login_url, sizeof(request.url) - 1);
    request.header_count = 1;
    strncpy(request.headers[0], "User-Agent: Mozilla/5.0", sizeof(request.headers[0]) - 1);
    strcpy(request.data, "");
    
    logger_info(mgr->logger, "Fetching CSRF token...");
    HTTPResponse *response = request_manager_send(req_mgr, &request);
    
    char *token = NULL;
    if (response) {
        if (response->status_code == 200) {
            token = csrf_manager_extract_from_html(response->body);
            if (token) {
                logger_info(mgr->logger, "CSRF token successfully fetched");
                csrf_manager_cache_token(mgr, token);
            } else {
                logger_error(mgr->logger, "Failed to extract CSRF token from response");
            }
        } else {
            logger_error(mgr->logger, "Failed to fetch CSRF token. HTTP %ld", response->status_code);
        }
        free(response);
    }
    
    request_manager_free(req_mgr);
    return token;
}

char* csrf_manager_refresh(CSRFManager *mgr, int use_proxy, const char *proxy) {
    logger_info(mgr->logger, "Refreshing CSRF token...");
    mgr->cache_count = 0;
    return csrf_manager_get_token(mgr, use_proxy, proxy);
}

int csrf_manager_validate(CSRFManager *mgr, const char *token, int use_proxy, const char *proxy) {
    RequestManager *req_mgr = request_manager_init(1.0, 3.0);
    if (!req_mgr) return 0;
    
    if (use_proxy && proxy) {
        request_manager_set_proxy(req_mgr, proxy);
    }
    
    HTTPRequest request = {0};
    strncpy(request.url, mgr->login_url, sizeof(request.url) - 1);
    request.header_count = 2;
    snprintf(request.headers[0], sizeof(request.headers[0]), "X-CSRFToken: %s", token);
    strncpy(request.headers[1], "User-Agent: Mozilla/5.0", sizeof(request.headers[1]) - 1);
    strcpy(request.data, "");
    
    HTTPResponse *response = request_manager_send(req_mgr, &request);
    
    int valid = 0;
    if (response) {
        valid = (response->status_code == 200);
        if (valid) {
            logger_info(mgr->logger, "CSRF token is valid");
        } else {
            logger_error(mgr->logger, "CSRF token validation failed. HTTP %ld", response->status_code);
        }
        free(response);
    }
    
    request_manager_free(req_mgr);
    return valid;
}
