#include "watchdog-box.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>

struct watchdog_box {
    volatile atomic_int click;  
};

watchdog_box* watchdog_box_create() {
    watchdog_box* box = malloc(sizeof(*box));

    if (!box) {
        exit(EXIT_FAILURE);
    }

    atomic_store(&box->click, 0);

    return box;
}

void watchdog_box_click(watchdog_box* box) {
    atomic_store(&box->click, 1);
}

int watchdog_box_check_click(watchdog_box* box) {
    return atomic_exchange(&box->click, 0);
}

void watchdog_box_destroy(watchdog_box* box) {
    free(box);
}
