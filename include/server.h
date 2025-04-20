#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>

// Starts the MUD server on the specified port.
// Returns true on success, false on failure.
bool server_start(int port);

#endif // SERVER_H
