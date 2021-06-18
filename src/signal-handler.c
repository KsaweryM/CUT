#include "signal-handler.h"
#include "macros.h"

volatile sig_atomic_t watchdog_object_exit = 0;

void signal_handler(int signum) {
    watchdog_object_exit = 1;
    UNUSED(signum);
}
