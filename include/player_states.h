// player_states.h
#ifndef PLAYER_STATES_H
#define PLAYER_STATES_H

typedef struct World World;
typedef struct GameRules GameRules;
typedef struct Player Player;

void player_state_enter_menu(GameRules *rules, World *world, Player *player);
void player_state_enter_playing(GameRules *rules, World *world, Player *player);
void player_state_enter_character_creation(GameRules *rules, World *world, Player *player);

#endif
