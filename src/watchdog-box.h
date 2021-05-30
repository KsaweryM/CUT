typedef struct watchdog_box watchdog_box;

watchdog_box* watchdog_box_create();

void watchdog_box_click(watchdog_box* box);

int watchdog_box_check_click(watchdog_box* box);

void watchdog_box_destroy(watchdog_box* box);
