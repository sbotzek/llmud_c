// player_states.h
#ifndef PLAYER_STATES_H
#define PLAYER_STATES_H

typedef struct World World;
typedef struct GameRules GameRules;
typedef struct Player Player;

// Start menu
void player_state_enter_menu(GameRules *rules, World *world, Player *player);

// Account creation / login / menu / selection
void player_state_enter_account_create(GameRules *rules, World *world, Player *player);
void player_state_enter_account_login(GameRules *rules, World *world, Player *player);
void player_state_enter_account_menu(GameRules *rules, World *world, Player *player);

// Character creation
void player_state_enter_character_creation(GameRules *rules, World *world, Player *player);

// Playing state
void player_state_enter_playing(GameRules *rules, World *world, Player *player, const char *name);

#endif // PLAYER_STATES_H
