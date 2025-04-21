#ifndef CLIENT_STATES_H
#define CLIENT_STATES_H

typedef struct World World;
typedef struct GameRules GameRules;
typedef struct Client Client;

void client_state_enter_menu(GameRules *rules, World *world, Client *client);
void client_state_enter_playing(GameRules *rules, World *world, Client *client);
void client_state_enter_character_creation(GameRules *rules, World *world, Client *client);

#endif
