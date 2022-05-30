#include <iostream>

#ifndef UNIX_SOCKET_H_
#define UNIX_SOCKET_H_

class Socket
{
public:
	Socket(ConstructorArgs);
	Socket(int);
	~Socket();

	auto set_option(Option) -> Socket&;
	auto bind(sockaddr_in&) -> Socket&;
	auto listen(const size_t con_number) -> Socket&;
	auto read(void* buffer, size_t length) -> Socket&;
	auto send(const std::string_view&) -> Socket&;
	auto accept(sockaddr_in&) -> Socket;
	auto errors() const -> const std::vector<std::string>&;

private:
	int m_fd;
	std::vector<std::string> m_errors;
};


#endif // UNIX_SOCKET_H_
