#pragma once

#include <stdbool.h>

#include "event.h"

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define RETURNS_NONNULL __attribute__((returns_nonnull))

void event_queue_init(void);
void event_queue_quit(void);
void event_queue_push(const Event *const event) NONNULL();
const Event *event_queue_get(void) RETURNS_NONNULL;
void event_queue_next(void);
bool event_queue_is_empty(void);
