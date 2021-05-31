#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#include "integer-buffer.h"
#include "watchdog-box.h"
#include <stdatomic.h>

typedef struct watchdog watchdog;

watchdog* watchdog_create(size_t box_length, watchdog_box* box[]);

void watchdog_send_exit_signal(watchdog* watchdog_object);

void watchdog_join(watchdog* watchdog_object);

void watchdog_destroy(watchdog* watchdog_object);

#endif //__WATCHDOG_H__