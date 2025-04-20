#ifndef CLIENT_STATES_H
#define CLIENT_STATES_H

#include "client.h"

void client_state_enter_menu(Client *client);
void client_state_enter_playing(Client *client);
void client_state_enter_character_creation(Client *client);

#endif // CLIENT_STATES_H

