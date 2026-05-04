#ifndef SECURITY_TESTER_H
#define SECURITY_TESTER_H

#include "logging.h"
#include "password_generator.h"
#include "request_manager.h"
#include "csrf_manager.h"
#include "monitoring.h"

#define SNAPCHAT_LOGIN_URL "https://www.snapchat.com/accounts/login/ajax/"
#define MAX_USERNAME_LENGTH 64
#define MAX_PASSWORD_LIST 10000
#define MAX_ATTEMPTS 500

typedef struct {
    Logger *logger;
    AIPasswordGenerator *generator;
    CSRFManager *csrf_mgr;
    OperationMonitor *monitor;
    RequestManager *request_mgr;
    int attempts;
    time_t start_time;
    char found_password[MAX_PASSWORD_LENGTH];
    int success;
} SecurityTester;

SecurityTester* security_tester_init(void);
void security_tester_free(SecurityTester *tester);
AttackResult security_tester_conduct_test(SecurityTester *tester, const char *username,
                                           char **password_list, int password_count,
                                           double delay, int use_proxy, const char *proxy);
int security_tester_test_credentials(SecurityTester *tester, const char *username,
                                      const char *password, const char *proxy);
void security_tester_save_results(SecurityTester *tester, const char *output_file,
                                   AttackResult *result, const char *username);

#endif
