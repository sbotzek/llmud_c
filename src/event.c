#include "event.h"
#include "macros.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct Subscription {
    int           id;
    bool          removed;
    void (*fn)(EventType, void *);
    struct Subscription *next;
} Subscription;

static Subscription *subscribers[MAX_EVENT] = { NULL };
static int next_subscribe_id = 1;

SubscribeToken event_subscribe(EventType type, void (*fn)(EventType, void *payload)) {
    CHECK_MSG(type >= EVENT_NONE && type < MAX_EVENT,
              "event_subscribe: invalid event type %d", type);

    Subscription *sub = malloc(sizeof *sub);
    CHECK_MSG(sub != NULL, "event_subscribe: malloc failed");

    sub->id      = next_subscribe_id++;
    sub->fn      = fn;
    sub->removed = false;
    sub->next    = NULL;

    Subscription **pp = &subscribers[type];
    if (*pp == NULL) {
        *pp = sub;
    } else {
        while ((*pp)->next) pp = &(*pp)->next;
        (*pp)->next = sub;
    }

    return (SubscribeToken) {
        .type = type,
        .id = sub->id
    };
}

void event_unsubscribe(SubscribeToken token) {
    CHECK_MSG(token.type >= EVENT_NONE && token.type < MAX_EVENT,
              "event_unsubscribe: invalid event type %d", token.type);
    CHECK_MSG(token.id > 0,
              "event_unsubscribe: invalid subscription id %d", token.id);

    // Mark matching subscription as removed
    for (Subscription *sub = subscribers[token.type]; sub; sub = sub->next) {
        if (sub->id == token.id) {
            sub->removed = true;
            return;
        }
    }
}

void event_emit(EventType type, void *payload) {
    CHECK_MSG(type >= EVENT_NONE && type < MAX_EVENT,
              "event_emit: invalid event type %d", type);

    for (Subscription *sub = subscribers[type]; sub; sub = sub->next) {
        if (!sub->removed) {
            sub->fn(type, payload);
        }
    }
}

typedef struct GameRules GameRules;
typedef struct World World;

void event_cleanup_tick(GameRules *rules, World *world) {
    UNUSED(rules);
    UNUSED(world);

    for (EventType type = EVENT_NONE; type < MAX_EVENT; ++type) {
        Subscription **pp = &subscribers[type];
        while (*pp) {
            if ((*pp)->removed) {
                Subscription *to_remove = *pp;
                *pp = to_remove->next;
                free(to_remove);
            } else {
                pp = &(*pp)->next;
            }
        }
    }
}
