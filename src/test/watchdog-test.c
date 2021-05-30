#include "watchdog-test.h"
#include "../watchdog.h"
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "../watchdog-box.h"

void* responding_thread(void* args) {
    watchdog_box* box = (watchdog_box*) args;


    for (int i = 0; i < 20; i++)  {
        printf("%d\n", i);

        if (i < 14) {
            watchdog_box_click(box);
        }

        sleep(1);
    }

    return 0;
}

void watchdog_test() {
    pthread_t id1, id2;

    watchdog_box* box1 = watchdog_box_create();
    watchdog_box* box2 = watchdog_box_create();

    watchdog_box_click(box1);
    watchdog_box_click(box2);

    pthread_create(&id1, NULL, &responding_thread, box1);
    pthread_create(&id2, NULL, &responding_thread, box2);

    watchdog_box* watchdog_input[] = {box1, box2};
    watchdog* watchdog_object = create_watchdog(2, watchdog_input);

    watchdog_join(watchdog_object);
}