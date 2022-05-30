#include "socket.hpp"
#include "utils.hpp"

#include <string>
#include <string_view>

void log_error(const std::string_view message)
{
	CLOG(message << " [" << std::to_string(WSAGetLastError()) << "]");
}

bool init_socket_lib()
{
	WSADATA wsa_data;

	auto wsa_started = WSAStartup(
		MAKEWORD(2,2), // use version 2.2 of Winsock
		&wsa_data
	);

	if (wsa_started != 0)
	{
		CLOG("couldnt start WSA");

		return false;
	}

	return true;
}

SOCKET create_server_socket()
{
	addrinfo* address_info;
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	auto getaddrinfo_result = getaddrinfo(NULL, "13908", &hints, &address_info);

	if (getaddrinfo_result != 0)
	{
		log_error("getaddrinfo failed");
		/* WSACleanup(); */

		return INVALID_SOCKET;
	}

	SOCKET sock = INVALID_SOCKET;

	sock = socket(
		address_info->ai_family,
		address_info->ai_socktype,
		address_info->ai_protocol
	);

	if (sock == INVALID_SOCKET)
	{
		log_error("Error at socket()");
		freeaddrinfo(address_info);
		/* WSACleanup(); */

		return INVALID_SOCKET;
	}

	// bind socket to ip address and port
	auto bind_result = bind(
		sock,
		address_info->ai_addr,
		(int)address_info->ai_addrlen
	);

    if (bind_result == SOCKET_ERROR)
	{
        log_error("bind failed with error");
        freeaddrinfo(address_info);
        closesocket(sock);
        /* WSACleanup(); */

		return INVALID_SOCKET;
    }

	freeaddrinfo(address_info);

	if (listen(sock, 64) == SOCKET_ERROR)
	{
		log_error( "Listen failed with error");
		closesocket(sock);
		/* WSACleanup(); */

		return INVALID_SOCKET;
	}

	return sock;
}
