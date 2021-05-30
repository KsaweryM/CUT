#include <stdio.h>
#include <signal.h>
#include "test/tests.h"
#include "reader.h"

reader* reader_object = 0;

void signal_handler() {
    if (reader_object) {
        reader_send_exit_signal(reader_object);
    }
}

int main() {
    signal(SIGTERM, signal_handler); 

    tests();
    
    return 0;
}