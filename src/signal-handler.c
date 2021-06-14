#include "signal-handler.h"

volatile sig_atomic_t watchdog_object_exit = 0;

static void signal_handler(int signum) {
    watchdog_object_exit = 1;
}

void signal_handler_create(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = signal_handler;
    sigaction(SIGINT, &action, NULL);
}
