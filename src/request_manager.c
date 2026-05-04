#include "request_manager.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#define sleep(seconds) Sleep((seconds) * 1000)
#endif

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    HTTPResponse *resp = (HTTPResponse *)userp;
    size_t total_size = size * nmemb;
    size_t remaining = MAX_RESPONSE_SIZE - resp->size - 1;
    
    if (total_size > remaining) {
        total_size = remaining;
    }
    
    memcpy(&(resp->body[resp->size]), contents, total_size);
    resp->size += total_size;
    resp->body[resp->size] = '\0';
    
    return total_size;
}

void rate_limiter_init(RateLimiter *limiter, int max_requests, int time_window) {
    limiter->count = 0;
    limiter->max_requests = max_requests;
    limiter->time_window = time_window;
    memset(limiter->records, 0, sizeof(limiter->records));
}

int rate_limiter_allow(RateLimiter *limiter) {
    time_t current = time(NULL);
    int valid_count = 0;
    
    for (int i = 0; i < limiter->count; i++) {
        if (current - limiter->records[i].timestamp < limiter->time_window) {
            valid_count += limiter->records[i].count;
        }
    }
    
    if (valid_count < limiter->max_requests) {
        if (limiter->count < MAX_REQUESTS_HISTORY) {
            limiter->records[limiter->count].timestamp = current;
            limiter->records[limiter->count].count = 1;
            limiter->count++;
        }
        return 1;
    }
    return 0;
}

double rate_limiter_get_delay(RateLimiter *limiter) {
    if (limiter->count == 0) return 0;
    
    time_t oldest = limiter->records[0].timestamp;
    for (int i = 1; i < limiter->count; i++) {
        if (limiter->records[i].timestamp < oldest) {
            oldest = limiter->records[i].timestamp;
        }
    }
    
    double elapsed = difftime(time(NULL), oldest);
    return limiter->time_window - elapsed;
}

RequestManager* request_manager_init(double min_delay, double max_delay) {
    RequestManager *mgr = (RequestManager*)malloc(sizeof(RequestManager));
    if (!mgr) return NULL;
    
    mgr->logger = logger_init("RequestManager", NULL);
    mgr->min_delay = min_delay;
    mgr->max_delay = max_delay;
    mgr->last_request_time = 0;
    mgr->use_proxy = 0;
    mgr->current_proxy[0] = '\0';
    
    rate_limiter_init(&mgr->limiter, 30, 60);
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    mgr->curl = curl_easy_init();
    
    return mgr;
}

void request_manager_free(RequestManager *mgr) {
    if (mgr) {
        if (mgr->curl) {
            curl_easy_cleanup(mgr->curl);
        }
        logger_free(mgr->logger);
        free(mgr);
        curl_global_cleanup();
    }
}

void request_manager_set_proxy(RequestManager *mgr, const char *proxy) {
    if (proxy && strlen(proxy) > 0) {
        strncpy(mgr->current_proxy, proxy, sizeof(mgr->current_proxy) - 1);
        mgr->current_proxy[sizeof(mgr->current_proxy) - 1] = '\0';
        mgr->use_proxy = 1;
        logger_info(mgr->logger, "Proxy set to: %s", proxy);
    }
}

void request_manager_enforce_delay(RequestManager *mgr) {
    if (mgr->last_request_time > 0) {
        double elapsed = difftime(time(NULL), mgr->last_request_time);
        if (elapsed < mgr->min_delay) {
            sleep((int)(mgr->min_delay - elapsed));
        }
    }
    
    double random_delay = ((double)rand() / RAND_MAX) * (mgr->max_delay - mgr->min_delay);
    if (random_delay > 0) {
        usleep((useconds_t)(random_delay * 1000000));
    }
}

int request_manager_handle_rate_limit(RequestManager *mgr, HTTPResponse *response) {
    if (response->status_code == 429) {
        int retry_after = 30;
        logger_warn(mgr->logger, "Rate limit hit. Waiting %d seconds...", retry_after);
        sleep(retry_after);
        return 1;
    }
    return 0;
}

HTTPResponse* request_manager_send(RequestManager *mgr, HTTPRequest *request) {
    HTTPResponse *response = (HTTPResponse*)calloc(1, sizeof(HTTPResponse));
    if (!response) return NULL;
    
    int max_retries = 3;
    int retry_count = 0;
    
    while (retry_count < max_retries) {
        if (!rate_limiter_allow(&mgr->limiter)) {
            double delay = rate_limiter_get_delay(&mgr->limiter);
            logger_debug(mgr->logger, "Rate limit reached, waiting %.2fs", delay);
            sleep((int)delay + 1);
        }
        
        request_manager_enforce_delay(mgr);
        
        if (mgr->curl) {
            curl_easy_cleanup(mgr->curl);
        }
        mgr->curl = curl_easy_init();
        
        curl_easy_setopt(mgr->curl, CURLOPT_URL, request->url);
        curl_easy_setopt(mgr->curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(mgr->curl, CURLOPT_WRITEDATA, (void *)response);
        curl_easy_setopt(mgr->curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(mgr->curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(mgr->curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(mgr->curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(mgr->curl, CURLOPT_USERAGENT,
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
        
        if (mgr->use_proxy && strlen(mgr->current_proxy) > 0) {
            curl_easy_setopt(mgr->curl, CURLOPT_PROXY, mgr->current_proxy);
        }
        
        struct curl_slist *header_list = NULL;
        for (int i = 0; i < request->header_count; i++) {
            header_list = curl_slist_append(header_list, request->headers[i]);
        }
        
        if (header_list) {
            curl_easy_setopt(mgr->curl, CURLOPT_HTTPHEADER, header_list);
        }
        
        if (strlen(request->data) > 0) {
            curl_easy_setopt(mgr->curl, CURLOPT_POSTFIELDS, request->data);
        }
        
        CURLcode res = curl_easy_perform(mgr->curl);
        
        if (res != CURLE_OK) {
            logger_error(mgr->logger, "Request failed: %s", curl_easy_strerror(res));
            retry_count++;
            if (retry_count < max_retries) {
                sleep(1 << retry_count);
            }
        } else {
            curl_easy_getinfo(mgr->curl, CURLINFO_RESPONSE_CODE, &response->status_code);
            
            char *ct;
            curl_easy_getinfo(mgr->curl, CURLINFO_CONTENT_TYPE, &ct);
            if (ct) {
                strncpy(response->content_type, ct, sizeof(response->content_type) - 1);
            }
            
            mgr->last_request_time = time(NULL);
            
            if (request_manager_handle_rate_limit(mgr, response)) {
                retry_count++;
                continue;
            }
            
            if (header_list) {
                curl_slist_free_all(header_list);
            }
            break;
        }
        
        if (header_list) {
            curl_slist_free_all(header_list);
        }
    }
    
    return response;
}
