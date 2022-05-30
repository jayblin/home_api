#include "socket-concept.hpp"
#include "utils.hpp"

#ifdef __unix__
	#include "unix/socket.hpp"
#elif defined(_WIN32) || defined(WIN32)
	#include "windows/socket.hpp"
#endif

#include <string_view>
#include <type_traits>
#include <vector>
#include <concepts>

#ifndef LOCAL_SOCKET_H_
#define LOCAL_SOCKET_H_

namespace jsocket
{

#ifdef __unix__
	using Socket = UnixSocket;
#elif defined(_WIN32) || defined(WIN32)
	using Socket = WindowsSocket;
#endif

}

static_assert(provides_errors<jsocket::Socket>);
static_assert(jsocket::is_socket<jsocket::Socket>);

#endif // LOCAL_SOCKET_H_
