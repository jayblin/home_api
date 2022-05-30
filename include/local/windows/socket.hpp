#include "socket-concept.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

class WindowsSocket
{
public:
	WindowsSocket(jsocket::ConstructorArgs);
	WindowsSocket(SOCKET);
	~WindowsSocket();

	auto set_option(jsocket::Option) -> WindowsSocket&;
	auto bind(jsocket::Address&) -> WindowsSocket&;
	auto listen(const size_t max_connections) -> WindowsSocket&;
	auto read(void* buffer, size_t buffer_length) -> WindowsSocket&;
	auto send(const std::string_view& response_data) -> WindowsSocket&;
	auto accept(jsocket::Address& addr) -> WindowsSocket;

	auto errors() const -> const std::vector<std::string>&
	{
		return m_errors;
	};

private:
	int m_domain;
	int m_socket_type;
	int m_protocol;
	addrinfo* m_address_info;
	SOCKET m_listen_socket;
	std::vector<std::string> m_errors;
};


