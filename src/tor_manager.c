#include "tor_manager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#define sleep(seconds) Sleep((seconds) * 1000)
#else
#include <unistd.h>
#endif

TorManager* tor_manager_init(void) {
    TorManager *mgr = (TorManager*)malloc(sizeof(TorManager));
    if (!mgr) return NULL;
    
    mgr->logger = logger_init("TorManager", NULL);
    mgr->last_identity_change = time(NULL);
    mgr->min_identity_change_interval = 30;
    mgr->control_password[0] = '\0';
    mgr->control_password_set = 0;
    
    return mgr;
}

void tor_manager_free(TorManager *mgr) {
    if (mgr) {
        logger_free(mgr->logger);
        free(mgr);
    }
}

void tor_manager_set_control_password(TorManager *mgr, const char *password) {
    if (password) {
        strncpy(mgr->control_password, password, MAX_TOR_PASSWORD - 1);
        mgr->control_password[MAX_TOR_PASSWORD - 1] = '\0';
        mgr->control_password_set = 1;
    }
}

static int run_command(const char *command) {
#ifdef _WIN32
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "cmd /c %s", command);
    
    if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 1;
    }
    return 0;
#else
    int result = system(command);
    return (result == 0);
#endif
}

int tor_manager_check_status(TorManager *mgr) {
    (void)mgr;
    const char *check_cmd = 
#ifdef _WIN32
        "curl --socks5-hostname 127.0.0.1:9050 https://check.torproject.org/ >nul 2>&1";
#else
        "curl --socks5-hostname 127.0.0.1:9050 https://check.torproject.org/ >/dev/null 2>&1";
#endif
    return run_command(check_cmd);
}

int tor_manager_ensure_running(TorManager *mgr) {
#ifdef _WIN32
    const char *check_tor = "tasklist | findstr tor.exe >nul";
#else
    const char *check_tor = "pgrep tor >/dev/null 2>&1";
#endif
    
    if (!run_command(check_tor)) {
        logger_info(mgr->logger, "Starting Tor service...");
        tor_manager_start(mgr);
        sleep(5);
    }
    
    if (!tor_manager_check_status(mgr)) {
        logger_warn(mgr->logger, "Tor not responding, restarting...");
        tor_manager_restart(mgr);
        sleep(5);
    }
    
    return tor_manager_check_status(mgr);
}

int tor_manager_change_identity(TorManager *mgr) {
    time_t current = time(NULL);
    if (current - mgr->last_identity_change < mgr->min_identity_change_interval) {
        return 0;
    }
    
#ifdef _WIN32
    const char *signal = "taskkill /F /IM tor.exe && tor.exe";
#else
    const char *signal = "killall -HUP tor";
#endif
    
    if (run_command(signal)) {
        sleep(3);
        mgr->last_identity_change = current;
        logger_info(mgr->logger, "Tor identity changed");
        return 1;
    }
    return 0;
}

int tor_manager_change_identity_via_control(TorManager *mgr) {
    time_t current = time(NULL);
    if (current - mgr->last_identity_change < mgr->min_identity_change_interval) {
        return 0;
    }
    
    if (!mgr->control_password_set) {
        logger_error(mgr->logger, "Control password not set");
        return 0;
    }
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "(echo authenticate \"%s\"; echo signal newnym; echo quit) | nc localhost %d",
        mgr->control_password, TOR_CONTROL_PORT);
    
    if (run_command(cmd)) {
        mgr->last_identity_change = current;
        logger_info(mgr->logger, "Tor identity changed via control port");
        return 1;
    }
    return 0;
}

void tor_manager_start(TorManager *mgr) {
#ifdef _WIN32
    const char *start_cmd = "start tor.exe";
#else
    const char *start_cmd = "sudo systemctl start tor || tor &";
#endif
    run_command(start_cmd);
    logger_info(mgr->logger, "Tor service started");
}

void tor_manager_stop(TorManager *mgr) {
#ifdef _WIN32
    const char *stop_cmd = "taskkill /F /IM tor.exe";
#else
    const char *stop_cmd = "sudo systemctl stop tor";
#endif
    run_command(stop_cmd);
    logger_info(mgr->logger, "Tor service stopped");
}

void tor_manager_restart(TorManager *mgr) {
    tor_manager_stop(mgr);
    sleep(2);
    tor_manager_start(mgr);
    logger_info(mgr->logger, "Tor service restarted");
}
