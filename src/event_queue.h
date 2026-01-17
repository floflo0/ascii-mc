#pragma once

#include <stdbool.h>

#include "event_defs.h"

void event_queue_init(void);

void event_queue_quit(void);

[[gnu::nonnull]]
void event_queue_push(const Event *const event);

[[gnu::returns_nonnull]]
const Event *event_queue_get(void);

void event_queue_next(void);

bool event_queue_is_empty(void);
