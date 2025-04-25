#include "test_event_queue.h"

#include "event_queue.h"
#include "test.h"

static void setup(void) {
    event_queue_init();
}

static void teardown(void) {
    event_queue_quit();
}

START_TEST(test_event_queue) {
    ck_assert(event_queue_is_empty());

    event_queue_push(&(Event){
        .type = EVENT_TYPE_CHAR,
        .char_event = {.chr = 'a'},
    });

    ck_assert(!event_queue_is_empty());

    event_queue_push(&(Event){
        .type = EVENT_TYPE_BUTTON_DOWN,
        .button_event = {.player_index = 1, .button = CONTROLLER_BUTTON_A},
    });

    ck_assert(!event_queue_is_empty());

    const Event *const event1 = event_queue_get();
    ck_assert_ptr_nonnull(event1);
    ck_assert_int_eq(event1->type, EVENT_TYPE_CHAR);
    ck_assert_int_eq(event1->char_event.chr, 'a');

    ck_assert(!event_queue_is_empty());

    // It should be the same as event1 because we don't call event_queue_next().
    const Event *const event2 = event_queue_get();
    ck_assert_ptr_nonnull(event2);
    ck_assert_int_eq(event2->type, EVENT_TYPE_CHAR);
    ck_assert_int_eq(event2->char_event.chr, 'a');

    ck_assert(!event_queue_is_empty());

    event_queue_next();

    ck_assert(!event_queue_is_empty());

    const Event *const event3 = event_queue_get();
    ck_assert_ptr_nonnull(event3);
    ck_assert_int_eq(event3->type, EVENT_TYPE_BUTTON_DOWN);
    ck_assert_int_eq(event3->button_event.player_index, 1);
    ck_assert_int_eq(event3->button_event.button, CONTROLLER_BUTTON_A);

    ck_assert(!event_queue_is_empty());

    event_queue_next();

    ck_assert(event_queue_is_empty());
}
END_TEST

// clang-format off
TEST_SUITE(
    event_queue,
    TEST_CASE_WITH_SETUP(
        "event_queue",
        TEST(test_event_queue),
        setup,
        teardown
    )
)
// clang-format on
