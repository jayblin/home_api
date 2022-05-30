#include "windows/socket.hpp"
#include "utils.hpp"

#include <string.h>
#include <iostream>

#include <string>
#include <winsock2.h>

#ifdef MSVC
#pragma comment (lib, "Ws2_32.lib")
#endif

auto convert_family(jsocket::AddressFamily family)
{
	switch (family)
	{
		case jsocket::AddressFamily::INET6:
			return AF_INET6;
		case jsocket::AddressFamily::INET:
			return AF_INET;
	}
}

auto convert_socket_type(jsocket::SocketType type)
{
	switch (type)
	{
		case jsocket::SocketType::RAW:
			return SOCK_RAW;
		case jsocket::SocketType::DGRAM:
			return SOCK_DGRAM;
		case jsocket::SocketType::STREAM:
			return SOCK_STREAM;
	}
}

int convert_protocol(jsocket::Protocol protocol)
{
	switch (protocol)
	{
		case jsocket::Protocol::TCP:
			return IPPROTO_TCP;
		case jsocket::Protocol::IPV4:
			return IPPROTO_IPV4;
		case jsocket::Protocol::DEFAULT:
			return 0;
	}
}

bool jsocket::init_socket_lib()
{
	/* initializing Winsock */

	WSADATA wsa_data;

	int i_result;

	i_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	if (i_result != 0)
	{
		CLOG("Couldn't initialize WinSock. Code:" << i_result);
		return false;
	}

	return true;
}

void jsocket::close_socket_lib()
{
	WSACleanup();
}

WindowsSocket::WindowsSocket(jsocket::ConstructorArgs args)
	:
	m_domain(convert_family(args.domain)),
	m_socket_type(convert_socket_type(args.type)),
	m_listen_socket(INVALID_SOCKET),
	m_protocol(convert_protocol(args.protocol)),
	m_address_info(NULL)
{}

WindowsSocket::WindowsSocket(SOCKET socket)
	: m_listen_socket(socket)
{}

WindowsSocket::~WindowsSocket()
{
	if (m_address_info)
	{
		freeaddrinfo(m_address_info);
	}

	if (m_listen_socket)
	{
		closesocket(m_listen_socket);
	}
}

static auto _bind = bind;
WindowsSocket& WindowsSocket::bind(jsocket::Address& address)
{
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = m_domain;
	hints.ai_socktype = m_socket_type;
	hints.ai_protocol = m_protocol;
	hints.ai_flags = AI_PASSIVE; // TODO

	auto i_result = getaddrinfo(
		NULL,
		address.port.data(),
		&hints,
		&m_address_info
	);

	if (i_result != 0)
	{
		m_errors.push_back(
			"[Error] WindowsSocket::bind() " + std::to_string(WSAGetLastError())
		);
	}


	m_listen_socket = socket(
		m_address_info->ai_family,
		m_address_info->ai_socktype,
		m_address_info->ai_protocol
	);

	if (m_listen_socket == INVALID_SOCKET)
	{
		m_errors.push_back(
			"[Error] WindowsSocket::bind() " + std::to_string(WSAGetLastError())
		);
	}

	i_result = _bind(
		m_listen_socket,
		m_address_info->ai_addr,
		(int) m_address_info->ai_addrlen
	);

	if (i_result == SOCKET_ERROR)
	{
		m_errors.push_back(
			"[Error] WindowsSocket::bind() " + std::to_string(WSAGetLastError())
		);
	}

	/* freeaddrinfo(result); */

	return *this;
}

static auto _accept = accept;
WindowsSocket WindowsSocket::accept(jsocket::Address &address)
{
	SOCKET client_socket;
	client_socket = INVALID_SOCKET;

	addrinfo hints;
	addrinfo* addr_info;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = m_domain;
	hints.ai_socktype = m_socket_type;
	hints.ai_protocol = m_protocol;
	hints.ai_flags = AI_PASSIVE; // TODO

	auto i_result = getaddrinfo(
		NULL,
		address.port.data(),
		&hints,
		&addr_info
	);

	if (i_result != 0)
	{
		m_errors.push_back(
			"[Error] WindowsSocket::bind() " + std::to_string(WSAGetLastError())
		);
	}

	/* const auto len = sizeof(hints); */

	client_socket = _accept(
		m_listen_socket,
		addr_info->ai_addr,
		(int*) &addr_info->ai_addrlen
		/* NULL, */
		/* NULL */
	);

	if (client_socket == INVALID_SOCKET)
	{
		m_errors.push_back(
			"[Error] WindowsSocket::accept() " + std::to_string(WSAGetLastError())
		);
	}
	
	return WindowsSocket{client_socket};
	/* return *this; */
}

static auto _send = send;
WindowsSocket& WindowsSocket::send(const std::string_view& response_data)
{
	int i_result;

	CLOG(response_data);

	auto i_send_result = _send(
		m_listen_socket,
		response_data.data(),
		i_result,
		0
	);

	if (i_send_result == SOCKET_ERROR)
	{
		m_errors.push_back(
			"[Error] WindowsSocket::send() " + std::to_string(WSAGetLastError())
		);
	}

	if (i_result == SOCKET_ERROR)
	{
		m_errors.push_back(
			"[Error] WindowsSocket::send() " + std::to_string(WSAGetLastError())
		);
	}

	return *this;
}

WindowsSocket& WindowsSocket::read(void* buffer, size_t buffer_length)
{
	/* auto str = std::string((char*)buffer, buffer_length); */

	/* CLOG("AAAAAA" << str << " : " << str.length()); */

	auto i_result = recv(
		m_listen_socket,
		(char*)buffer,
		buffer_length,
		0
	);

	if (i_result == SOCKET_ERROR)
	{
		m_errors.push_back(
			"[Error] WindowsSocket::read() " + std::to_string(WSAGetLastError())
		);
	}

	return *this;
}

static auto _listen = listen;
WindowsSocket& WindowsSocket::listen(const size_t max_connections)
{
	if (_listen(m_listen_socket, max_connections) == SOCKET_ERROR)
	{
		m_errors.push_back(
			"[Error] WindowsSocket::listen() " + std::to_string(WSAGetLastError())
		);
	}

	return *this;
}

static auto convert_option_level(jsocket::Option& option)
{
	switch (option.level)
	{
		case jsocket::OptionLevel::SOCKET:
			return SOL_SOCKET;
	}
}

static auto convert_option_name(jsocket::Option& option)
{
	switch (option.name)
	{
		case jsocket::OptionName::REUSEADDR:
			return SO_REUSEADDR;
		case jsocket::OptionName::KEEPALIVE:
			return SO_KEEPALIVE;
	}
}

WindowsSocket& WindowsSocket::set_option(jsocket::Option option)
{
	if (option.name == jsocket::OptionName::REUSEPORT) {
		return *this;
	}

	auto result = setsockopt(
		m_listen_socket,
		convert_option_level(option),
		convert_option_name(option),
		(char*) &option.value,
		sizeof(option.value)
	);

	if (result == -1)
	{
		m_errors.push_back(
			"[Error] WindowsSocket::set_option() " + std::to_string(WSAGetLastError())
		);
	}

	return *this;
}

/* WindowsSocket& WindowsSocket::set_option(jsocket::Option option) */
/* { */
/* 	if (option.name == jsocket::OptionName::REUSEPORT) { */
/* 		return *this; */
/* 	} */

/* 	auto result = setsockopt( */
/* 		m_listen_socket, */
/* 		convert_option_level(option), */
/* 		convert_option_name(option), */
/* 		(char*) &option.value, */
/* 		sizeof(option.value) */
/* 	); */

/* 	if (result == -1) */
/* 	{ */
/* 		m_errors.push_back( */
/* 			"[Error] WindowsSocket::set_option() " + std::to_string(WSAGetLastError()) */
/* 		); */
/* 	} */

/* 	return *this; */
/* } */
