#include "watchdog.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "signal-handler.h"

struct watchdog {
    atomic_int exit;
    atomic_int* program_exit;
    pthread_t id;
    size_t box_length;
    watchdog_box* box[];
};

void* thread_watchdog(void* args);

void* thread_watchdog(void* args) {
    watchdog* watchdog_object = (watchdog*) args;
    register const int watchdog_sleep_inteveral = 2;

    while (!watchdog_object_exit) {
        sleep(watchdog_sleep_inteveral);

        for (size_t i = 0; i < watchdog_object->box_length; i++) {
            
            int click = watchdog_box_check_click(watchdog_object->box[i]);

            if (!click) {
                // the only way to exit the program when one of the threads crash and does not respond
                printf("watchdog - box %d\n", (int) i);
                exit(EXIT_FAILURE);
            }
        }
    }
   
    printf("watchdog exit\n");

    return 0;
}

watchdog* watchdog_create(size_t box_length, watchdog_box* box[]) {
    watchdog* watchdog_object = malloc(sizeof(*watchdog_object) + sizeof(watchdog_box*) * box_length);

    memcpy(watchdog_object->box, box, sizeof(watchdog_box*) * box_length);
    watchdog_object->exit = 0;
    watchdog_object->box_length = box_length;
    
   pthread_create(&watchdog_object->id, NULL, &thread_watchdog, watchdog_object);

    return watchdog_object;
}

void watchdog_send_exit_signal(watchdog* watchdog_object) {
    watchdog_object->exit = 1;
}

void watchdog_join(watchdog* watchdog_object) {
    pthread_join(watchdog_object->id, NULL);
}

void watchdog_destroy(watchdog* watchdog_object) {
    free(watchdog_object);
}
