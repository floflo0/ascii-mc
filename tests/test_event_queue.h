#include <check.h>

#define RETURNS_NONNULL __attribute__((returns_nonnull))

Suite *event_queue_suite(void) RETURNS_NONNULL;
