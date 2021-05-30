#include "integer-buffer-test.h"
#include "string-buffer-test.h"
#include "reader-test.h"
#include "tests.h"
#include "watchdog-test.h"

void tests() {
    watchdog_test();
    reader_test();
    integer_buffer_test();
    string_buffer_test();   
}