// act.h
#ifndef ACT_H
#define ACT_H
#include <stdbool.h>
#include "actor_id.h"

typedef struct Actor Actor;

// All acts are declared in this enum.
typedef enum ActType {
    ACT_NONE = 0,

    ACT_MOVE,

    ACT_EXAMINE_LOCATION,
    ACT_EXAMINE_DIRECTION,

    ACT_MAX,
} ActType;

typedef enum ActPhase {
    ACT_PHASE_PERFORM,
    ACT_PHASE_RESOLVE,
} ActPhase;

typedef struct Act Act;

// Function types used for defining an act.
typedef bool (*ActPerformFn)(Act *act);
typedef void (*ActPerceiveFn)(Act *act, Actor *viewer);

// Should be the first member of your act-specific struct.
typedef struct Act {
    ActType type; // must be set

    ActPerformFn perform_fn; // nullable
    ActPerceiveFn ai_perceive_fn; // nullable
    ActPerceiveFn player_perceive_fn; // nullable

    Actor *actor; // not null
} Act;

typedef enum ActListenerResult {
    ACT_LISTENER_CONTINUE, // continue normal processing
    ACT_LISTENER_CANCEL, // stop running, consider act not performed
    ACT_LISTENER_ACCEPT, // stop running, consider act performed
} ActListenerResult;

// Function type used to listen to an act occurring.  Result only honored in ACT_PHASE_PERFORM.
typedef ActListenerResult (*ActListenerFn)(ActPhase phase, Act *act);

// Runs an act through all phases.  Returns true when the act was able to be performed.
bool act_run(Act *act);

// Perceives the act to the viewer.
void act_perceive_to(Act *act, Actor *viewer);
// Perceives the act to the actor's location and all actors in it.
void act_perceive_at(Act *act);
// Like act_perceive_at, but will exclude actors in exclude (NULL terminated array).
void act_perceive_at_except(Act *act, Actor *exclude[]);
// Perceives the act to the location and all actors in it.
void act_perceive_location(Act *act, ActorID location_id);
// Like act_perceive_location, but will exclude actors in exclude (NULL terminated array).
void act_perceive_location_except(Act *act, ActorID location_id, Actor *exclude[]);

// Register a listener for the given act type.  The function is called in each phase.
void act_register_listener(ActType type, ActListenerFn fn);

#endif //ACT_H
