#ifndef EVENT_H
#define EVENT_H

typedef enum {
    EVENT_NONE = 0,
    EVENT_SOCKET_CONNECT,
    MAX_EVENT
} EventType;

typedef struct SubscribeToken {
    EventType type;
    int id;
} SubscribeToken;

extern const SubscribeToken INVALID_SUBSCRIBE_TOKEN;

SubscribeToken event_subscribe(EventType type, void (*fn)(EventType type, void* payload));
void event_unsubscribe(SubscribeToken token);
void event_emit(EventType type, void *payload);
void event_cleanup_subscriptions(void);

#endif // EVENT_H
