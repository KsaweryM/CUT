#include "watchdog-box.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>

struct watchdog_box {
    pthread_mutex_t mutex;
    char padding[4];
    int click;  
};

watchdog_box* watchdog_box_create() {
    watchdog_box* box = malloc(sizeof(*box));

    if (!box) {
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&box->mutex, NULL);

    box->click = 0;

    return box;
}

void watchdog_box_click(watchdog_box* box) {
    pthread_mutex_lock(&box->mutex);

    box->click = 1;

    pthread_mutex_unlock(&box->mutex);
}

int watchdog_box_check_click(watchdog_box* box) {
    int click;

    pthread_mutex_lock(&box->mutex);
    // fetch and exchange
    click = box->click;
    box->click = 0;

    pthread_mutex_unlock(&box->mutex);

    return click;
}

void watchdog_box_destroy(watchdog_box* box) {
    pthread_mutex_destroy(&box->mutex);

    free(box);
}
