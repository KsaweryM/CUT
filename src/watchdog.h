#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "integer-buffer.h"
#include "watchdog-box.h"
#include <stdatomic.h>

/*
* Watchdog ensures that the program does not freeze.
* Each thread needs to send it a message at least every 2 seconds to let it know that it is working properly. 
* If the thread is not informed in time that every thread is working correctly,
* it aborts the program with an error message. 
* 
* Every thread in the program has a watchdog_box object.
* To inform watchdog about thread activity, every thread must execute watchdog_box_click(watchdog_box) at least every 2 seconds.
*/
typedef struct watchdog watchdog;

/*
* The first two function's arguments are arrray of watchdog_box pointers and length of this array.
* Watchdog_box object are used to inform watchdog about thread activity. Detailed description of watchdog_box is in the comment above.
*/
watchdog* watchdog_create(size_t box_length, watchdog_box* box[]);

void watchdog_join(watchdog* watchdog_object);

void watchdog_destroy(watchdog* watchdog_object);

#endif 
