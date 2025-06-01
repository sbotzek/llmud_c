// act.h
#ifndef ACT_H
#define ACT_H
#include "actor.h"

// All acts are declared in this enum.
typedef enum ActType {
    ACT_NONE = 0,
    ACT_GO_DIRECTION,
    MAX_ACT,
} ActType;

typedef enum ActPhase {
    ACT_PHASE_PREPARE,
    ACT_PHASE_COMMIT,
    ACT_PHASE_PERCEIVE,
} ActPhase;

// Function types used for defining an act.
typedef bool (*ActPrepareFn)(Actor *actor, void *act);
typedef void (*ActCommitFn)(Actor *actor, void *act);
typedef void (*ActPerceiveFn)(Actor *viewer, Actor *actor, void *act);

// Function type used to listen to an act occurring.
typedef bool (*ActListenerFn)(ActPhase phase, ActType type, Actor *actor, void *act);

// Executes an act through all phases.
void act_execute(ActType type, Actor *actor, void *act);

// Configures various handlers for an act.  All functions optional.
void act_configure(ActType type, ActPrepareFn prepare_fn, ActCommitFn commit_fn, ActPerceiveFn player_fn, ActPerceiveFn ai_fn);
// Register a listener for the given act type.  The function is called in each phase.
void act_register_listener(ActType type, ActListenerFn fn);

#endif //ACT_H
