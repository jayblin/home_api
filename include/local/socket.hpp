#ifndef SOCKET_H_
#define SOCKET_H_

#include <string_view>
#include <winsock2.h>
#include <ws2tcpip.h>

void log_error(const std::string_view message);

bool init_socket_lib();

SOCKET create_server_socket();

#endif // SOCKET_H_
