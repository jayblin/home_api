#include "utils.hpp"

#include <string_view>
#include <type_traits>
#include <vector>
#include <concepts>

#ifndef LOCAL_SOCKET_CONCEPT_H_
#define LOCAL_SOCKET_CONCEPT_H_

namespace jsocket
{

	enum class AddressFamily
	{
		INET,
		INET6,
	};

	enum class SocketType
	{
		STREAM,
		DGRAM,
		RAW,
	};

	enum class BindingAddress
	{
		ANY,
		LOOPBACK,
	};

	enum class Protocol
	{
		IPV4,
		TCP,
		DEFAULT,
	};

	enum class OptionLevel
	{
		SOCKET,
	};

	enum class OptionName
	{
		REUSEADDR,
		REUSEPORT,
		KEEPALIVE,
	};

	struct ConstructorArgs
	{
		AddressFamily domain;
		SocketType type;
		Protocol protocol;
	};

	struct Option
	{
		OptionLevel level;
		OptionName name;
		int value;
	};

	struct Address
	{
		AddressFamily family;
		std::string port;
		BindingAddress addr;
	};

	template <class T>
	concept is_socket = requires(
		T t,
		Address& addr,
		const size_t max_connections,
		void* buffer,
		size_t buffer_length,
		const std::string_view& response_data
		/* Option& option_ref_type */
	)
	{
		{ T(ConstructorArgs()) };
		{ t.set_option(Option()) } -> std::same_as<T&>;
		/* { t.set_option(option_ref_type) } -> std::same_as<T&>; */
		{ t.bind(addr) } -> std::same_as<T&>;
		{ t.listen(max_connections) } -> std::same_as<T&>;
		{ t.read(buffer, buffer_length) } -> std::same_as<T&>;
		{ t.send(response_data) } -> std::same_as<T&>;
		{ t.accept(addr) } -> std::same_as<T>;
	};

	bool init_socket_lib();
	void close_socket_lib();
}

#endif // LOCAL_SOCKET_CONCEPT_H_
