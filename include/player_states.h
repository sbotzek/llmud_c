// player_states.h
#ifndef PLAYER_STATES_H
#define PLAYER_STATES_H

typedef struct GameRules GameRules;
typedef struct Player Player;

// Start menu
void player_state_enter_menu(Player *player);

// Account creation / login / menu / selection
void player_state_enter_account_create(Player *player);
void player_state_enter_account_login(Player *player);
void player_state_enter_account_menu(Player *player);

// Character creation
void player_state_enter_character_creation(Player *player);

// Playing state
void player_state_enter_playing(Player *player, const char *name);

#endif // PLAYER_STATES_H
