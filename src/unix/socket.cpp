#include "socket.hpp"

#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


Socket::Socket(ConstructorArgs args)
	: m_fd(socket(
		args.domain,
		args.type,
		args.protocol
	))
{
}

Socket::Socket(int fd)
	: m_fd(fd)
{
}

Socket::~Socket()
{
	close(m_fd);
}

Socket& Socket::set_option(Option option)
{
	auto opt = option.value;

	const auto set_opt_result = setsockopt(
		m_fd,
		SOL_SOCKET,
		SO_REUSEADDR | SO_REUSEPORT,
		&opt,
		sizeof(opt)
	);

	if (set_opt_result == -1)
	{
		m_errors.push_back(
			"[Error] Socket::set_option() " + std::string{strerror(errno)}
		);
	}

	return *this;
}

static auto _bind = bind;
Socket& Socket::bind(sockaddr_in& address)
{
	const auto bind_result = _bind(
		m_fd,
		reinterpret_cast<sockaddr *>(&address),
		sizeof(address)
	);

	if (bind_result == -1)
	{
		m_errors.push_back(
			"[Error] Socket::bind() " + std::string{strerror(errno)}
		);
	}

	return *this;
}

static auto _listen = listen;
Socket& Socket::listen(const size_t con_number)
{
	const auto listen_result = _listen(m_fd, con_number);

	if (listen_result == -1)
	{
		m_errors.push_back(
			"[Error] Socket::listen() " + std::string{strerror(errno)}
		);
	}

	return *this;
}

static auto _accept = accept;
Socket Socket::accept(sockaddr_in& address)
{
	auto address_length = sizeof(address);

	return _accept(
		m_fd,
		reinterpret_cast<sockaddr *>(&address),
		reinterpret_cast<socklen_t *>(&address_length)
	);
}

static const auto _read = read;
Socket& Socket::read(void* buffer, size_t length)
{
	const auto read_result = _read(m_fd, buffer, length);

	return *this;
}

static const auto _send = send;
Socket& Socket::send(const std::string_view& data)
{
	_send(m_fd, data.data(), data.length(), 0);

	return *this;
}

const std::vector<std::string>& Socket::errors()
{
	return m_errors;
}

