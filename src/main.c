/*
 * Recover-Snapchat-Account-AI
 * C Implementation - Version 2026.1
 * Educational Security Testing Tool
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "logging.h"
#include "password_generator.h"
#include "request_manager.h"
#include "proxy_manager.h"
#include "tor_manager.h"
#include "csrf_manager.h"
#include "monitoring.h"
#include "security_tester.h"

#define VERSION "2026.1"
#define MAX_PASSWORDS 10000
#define MAX_LINE_LENGTH 256

typedef struct {
    char username[MAX_LINE_LENGTH];
    char password_list[MAX_LINE_LENGTH];
    char output_file[MAX_LINE_LENGTH];
    char proxy_list[MAX_LINE_LENGTH];
    int use_tor;
    int threads;
    double min_delay;
    double max_delay;
    int timeout;
} Config;

static void print_banner(void) {
    printf("\033[1;32m\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     Recover-Snapchat-Account-AI                        ║\n");
    printf("║     C Implementation - Version %s                      ║\n", VERSION);
    printf("║     Security Testing Tool - Educational Use Only        ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\033[1;37m\n");
}

static void print_usage(const char *prog) {
    printf("\033[1;34m\n");
    printf("Usage: %s [OPTIONS]\n\n", prog);
    printf("Required Options:\n");
    printf("  --username <name>       Target username for security testing\n");
    printf("  --password-list <file>  Path to password list file\n\n");
    printf("Optional Options:\n");
    printf("  --proxy-list <file>     Path to proxy list file\n");
    printf("  --use-tor               Enable Tor for anonymous testing\n");
    printf("  --threads <num>         Number of threads (default: 1)\n");
    printf("  --min-delay <sec>       Minimum delay between requests (default: 1.0)\n");
    printf("  --max-delay <sec>       Maximum delay between requests (default: 3.0)\n");
    printf("  --output <file>         Output file for results (default: security_results.txt)\n");
    printf("  --timeout <sec>         Request timeout in seconds (default: 3600)\n");
    printf("  --help                  Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s --username test_user --password-list passwords.txt\n", prog);
    printf("  %s --username test_user --password-list passwords.txt --use-tor\n", prog);
    printf("  %s --username test_user --password-list passwords.txt --proxy-list proxies.txt\n", prog);
    printf("\033[0m\n");
}

static int parse_args(int argc, char **argv, Config *config) {
    strncpy(config->output_file, "security_results.txt", sizeof(config->output_file) - 1);
    config->use_tor = 0;
    config->threads = 1;
    config->min_delay = 1.0;
    config->max_delay = 3.0;
    config->timeout = 3600;
    config->proxy_list[0] = '\0';
    config->password_list[0] = '\0';
    config->username[0] = '\0';
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return -1;
        } else if (strcmp(argv[i], "--username") == 0 && i + 1 < argc) {
            strncpy(config->username, argv[++i], sizeof(config->username) - 1);
            config->username[sizeof(config->username) - 1] = '\0';
        } else if (strcmp(argv[i], "--password-list") == 0 && i + 1 < argc) {
            strncpy(config->password_list, argv[++i], sizeof(config->password_list) - 1);
            config->password_list[sizeof(config->password_list) - 1] = '\0';
        } else if (strcmp(argv[i], "--proxy-list") == 0 && i + 1 < argc) {
            strncpy(config->proxy_list, argv[++i], sizeof(config->proxy_list) - 1);
            config->proxy_list[sizeof(config->proxy_list) - 1] = '\0';
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            strncpy(config->output_file, argv[++i], sizeof(config->output_file) - 1);
            config->output_file[sizeof(config->output_file) - 1] = '\0';
        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            config->threads = atoi(argv[++i]);
            if (config->threads < 1) config->threads = 1;
        } else if (strcmp(argv[i], "--min-delay") == 0 && i + 1 < argc) {
            config->min_delay = atof(argv[++i]);
        } else if (strcmp(argv[i], "--max-delay") == 0 && i + 1 < argc) {
            config->max_delay = atof(argv[++i]);
        } else if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            config->timeout = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--use-tor") == 0) {
            config->use_tor = 1;
        }
    }
    
    if (strlen(config->username) == 0) {
        fprintf(stderr, "Error: Username is required\n");
        return 0;
    }
    if (strlen(config->password_list) == 0) {
        fprintf(stderr, "Error: Password list is required\n");
        return 0;
    }
    
    return 1;
}

static int load_passwords(const char *filename, char ***passwords, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open password list file: %s\n", filename);
        return 0;
    }
    
    *passwords = (char**)malloc(sizeof(char*) * MAX_PASSWORDS);
    if (!*passwords) {
        fclose(file);
        return 0;
    }
    
    char line[MAX_PASSWORD_LENGTH];
    *count = 0;
    
    while (fgets(line, sizeof(line), file) && *count < MAX_PASSWORDS) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }
        if (len > 0 && line[len - 1] == '\r') {
            line[len - 1] = '\0';
            len--;
        }
        if (len > 0) {
            (*passwords)[*count] = strdup(line);
            (*count)++;
        }
    }
    
    fclose(file);
    printf("Loaded %d passwords from %s\n", *count, filename);
    return 1;
}

static void free_passwords(char **passwords, int count) {
    for (int i = 0; i < count; i++) {
        free(passwords[i]);
    }
    free(passwords);
}

int main(int argc, char **argv) {
    Config config;
    
    print_banner();
    
    int parse_result = parse_args(argc, argv, &config);
    if (parse_result < 0) {
        return 0;
    }
    if (parse_result == 0) {
        print_usage(argv[0]);
        return 1;
    }
    
    char **passwords = NULL;
    int password_count = 0;
    
    if (!load_passwords(config.password_list, &passwords, &password_count)) {
        return 1;
    }
    
    printf("\n🔒 SECURITY DISCLAIMER:\n");
    printf("🔒 This tool is for educational and authorized security testing only\n");
    printf("🔒 Unauthorized use may violate laws and terms of service\n");
    printf("🔒 You are responsible for ensuring proper authorization\n\n");
    
    printf("Type 'AUTHORIZED' to continue: ");
    char confirm[64];
    if (scanf("%63s", confirm) != 1 || strcmp(confirm, "AUTHORIZED") != 0) {
        printf("Operation cancelled - Authorization required\n");
        free_passwords(passwords, password_count);
        return 0;
    }
    
    SecurityTester *tester = security_tester_init();
    if (!tester) {
        fprintf(stderr, "Failed to initialize security tester\n");
        free_passwords(passwords, password_count);
        return 1;
    }
    
    char *proxy = NULL;
    ProxyManager *proxy_mgr = NULL;
    TorManager *tor_mgr = NULL;
    
    if (strlen(config.proxy_list) > 0) {
        printf("Loading proxies from %s...\n", config.proxy_list);
        proxy_mgr = proxy_manager_init(config.proxy_list, NULL);
        if (proxy_mgr) {
            proxy_manager_load_from_file(proxy_mgr);
            proxy_manager_validate_all(proxy_mgr);
            proxy = proxy_manager_get_random(proxy_mgr);
            if (proxy) {
                printf("Using proxy: %s\n", proxy);
            }
        }
    }
    
    if (config.use_tor) {
        printf("Configuring Tor for anonymous testing...\n");
        tor_mgr = tor_manager_init();
        if (tor_mgr) {
            if (tor_manager_ensure_running(tor_mgr)) {
                printf("Tor anonymity enabled\n");
                proxy = "socks5://127.0.0.1:9050";
            } else {
                fprintf(stderr, "Failed to initialize Tor. Exiting...\n");
                security_tester_free(tester);
                free_passwords(passwords, password_count);
                if (proxy_mgr) proxy_manager_free(proxy_mgr);
                if (tor_mgr) tor_manager_free(tor_mgr);
                return 1;
            }
        }
    }
    
    printf("\nStarting security assessment for user: %s\n", config.username);
    
    AttackResult result = security_tester_conduct_test(
        tester, config.username, passwords, password_count,
        config.min_delay, proxy != NULL, proxy
    );
    
    security_tester_save_results(tester, config.output_file, &result, config.username);
    
    printf("\n========================================\n");
    if (result.success) {
        printf("SECURITY ALERT: Weak password detected!\n");
        printf("Password found: %s\n", result.password);
        printf("Attempts: %d\n", result.attempts);
        printf("Duration: %.2f seconds\n", result.duration);
    } else {
        printf("Security assessment completed - no issues detected\n");
        printf("Tests conducted: %d\n", result.attempts);
        printf("Duration: %.2f seconds\n", result.duration);
    }
    printf("========================================\n");
    
    security_tester_free(tester);
    free_passwords(passwords, password_count);
    if (proxy_mgr) proxy_manager_free(proxy_mgr);
    if (tor_mgr) tor_manager_free(tor_mgr);
    
    return result.success ? 0 : 1;
}
