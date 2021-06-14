#include "signal-handler.h"

#define UNUSED(x) (void)(x)

volatile sig_atomic_t watchdog_object_exit = 0;

static void signal_handler(int signum) {
    UNUSED(signum);
    watchdog_object_exit = 1;
}

struct sigaction signal_handler_create(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = signal_handler;
    sigaction(SIGTERM, &action, NULL);

    return action;
}
