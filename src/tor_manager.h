#ifndef TOR_MANAGER_H
#define TOR_MANAGER_H

#include "logging.h"
#include <time.h>

#define TOR_CONTROL_PORT 9051
#define TOR_SOCKS_PORT 9050
#define MAX_TOR_PASSWORD 256

typedef struct {
    Logger *logger;
    time_t last_identity_change;
    int min_identity_change_interval;
    char control_password[MAX_TOR_PASSWORD];
    int control_password_set;
} TorManager;

TorManager* tor_manager_init(void);
void tor_manager_free(TorManager *mgr);
int tor_manager_ensure_running(TorManager *mgr);
int tor_manager_check_status(TorManager *mgr);
int tor_manager_change_identity(TorManager *mgr);
int tor_manager_change_identity_via_control(TorManager *mgr);
void tor_manager_start(TorManager *mgr);
void tor_manager_stop(TorManager *mgr);
void tor_manager_restart(TorManager *mgr);
void tor_manager_set_control_password(TorManager *mgr, const char *password);

#endif
