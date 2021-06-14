#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
 
extern volatile sig_atomic_t watchdog_object_exit;

void signal_handler_create(void);

#endif
