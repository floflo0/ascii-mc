#include <assert.h>
#include <stdlib.h>

#include "test_event_queue.h"
#include "test_viewport.h"

int main(void) {
    SRunner *const suite_runner = srunner_create(NULL);
    assert(suite_runner != NULL);

    srunner_add_suite(suite_runner, event_queue_suite());
    srunner_add_suite(suite_runner, viewport_suite());

    srunner_run_all(suite_runner, CK_NORMAL);
    const int number_tests_failed = srunner_ntests_failed(suite_runner);
    srunner_free(suite_runner);
    return number_tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
