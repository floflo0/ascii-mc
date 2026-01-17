#include "event_queue.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "gamepad.h"
#include "threads.h"
#include "utils.h"

typedef struct EventQueueNode {
    struct EventQueueNode *next;
    Event event;
} EventQueueNode;

typedef struct {
    EventQueueNode *first;
    EventQueueNode *last;
#ifndef __wasm__
    pthread_mutex_t mutex;
#endif
} EventQueue;

static EventQueue event_queue = {
    .first = NULL,
    .last = NULL,
#ifndef __wasm__
    .mutex = PTHREAD_MUTEX_INITIALIZER,
#endif
};

#ifndef NDEBUG
static bool event_queue_is_init = false;
#endif

void event_queue_init(void) {
    assert(!event_queue_is_init);
    event_queue.last = event_queue.first;
#ifndef NDEBUG
    event_queue_is_init = true;
#endif
}

void event_queue_quit(void) {
    assert(event_queue_is_init);
    while (event_queue.first != NULL) {
        EventQueueNode *const first = event_queue.first;
        event_queue.first = event_queue.first->next;
        if (first->event.type == EVENT_TYPE_GAMEPAD_CONNECT ||
            first->event.type == EVENT_TYPE_GAMEPAD_DISCONNECT) {
            gamepad_destroy(first->event.gamepad_event.gamepad);
        }
        free(first);
    }
#ifndef NDEBUG
    event_queue_is_init = false;
#endif
}

void event_queue_push(const Event *const event) {
    assert(event_queue_is_init);
    assert(event != NULL);
    EventQueueNode *node =
        malloc_or_exit(sizeof(*node), "failed to push event in queue");

    memcpy(&node->event, event, sizeof(*event));
    node->next = NULL;

    mutex_lock(&event_queue.mutex);
    if (event_queue.first != NULL) {
        event_queue.last->next = node;
    } else {
        event_queue.first = node;
    }
    event_queue.last = node;
    mutex_unlock(&event_queue.mutex);
}

inline const Event *event_queue_get(void) {
    assert(event_queue_is_init);
    assert(event_queue.first != NULL);
    assert(&event_queue.first->event != NULL);
    return &event_queue.first->event;
}

void event_queue_next(void) {
    assert(event_queue_is_init);
    mutex_lock(&event_queue.mutex);
    EventQueueNode *const first = event_queue.first;
    event_queue.first = first->next;
    mutex_unlock(&event_queue.mutex);
    free(first);
}

inline bool event_queue_is_empty(void) {
    assert(event_queue_is_init);
    return event_queue.first == NULL;
}
