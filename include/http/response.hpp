#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "utils.hpp"

#include <cstring>
#include <string>
#include <string_view>
#include <utility>

namespace http
{
	struct Response
	{
	public:
		/* static constexpr auto BODY_BUFFER_SIZE = 1024 * 1024; */

		/* const http::Code code = http::Code::OK; */
		/* /1* const std::string content = ""; *1/ */
		/* const http::ContentType content_type = http::ContentType::TEXT_HTML; */
		/* const http::Charset charset = http::Charset::UTF_8; */
		/* const char content[BODY_BUFFER_SIZE]; */
		auto code(http::Code value) -> Response&
		{
			m_code = value;
			return *this;
		}
		auto content_type(http::ContentType value) -> Response&
		{
			m_content_type = value;
			return *this;
		}
		auto charset(http::Charset value) -> Response&
		{
			m_charset = value;
			return *this;
		}
		/* auto content(std::string_view&& value) -> Response& */
		auto content(std::string&& value) -> Response&
		{
			m_content = value;
			return *this;
		}
		/* /1* auto content(const char* value) -> Response& *1/ */
		/* { */
		/* 	/1* this->m_content = value; *1/ */
		/* 	/1* this->m_content = std::move(value); *1/ */
		/* 	/1* std::memcpy( *1/ */
		/* 	CLOG(value); */
		/* 	std::memcpy(m_content, value.data(), value.length()); */

		/* 	return *this; */
		/* } */

		auto code() const -> http::Code { return m_code; };
		auto content_type() const -> http::ContentType { return m_content_type; };
		auto charset() const -> http::Charset { return m_charset; };
		/* auto content() const -> const char* { return m_content; }; */
		auto content() const -> const std::string& { return m_content; };

		/* Response() */
		/* {} */

		/* Response(http::Code code) */
		/* 	: m_code(code) */
		/* {} */

		/* Response() : */
		/* 	m_code(http::Code::OK), */
		/* 	m_content_type(http::ContentType::TEXT_HTML), */
		/* 	m_charset(http::Charset::UTF_8) */
		/* { */
		/* 	CLOG("MAKING"); */
		/* 	/1* std::memset(this->m_content, 0, BODY_BUFFER_SIZE); *1/ */
		/* } */

		/* Response(const Response& other) */
		/* 	: */
		/* 	m_code(other.m_code), */
		/* 	m_charset(other.m_charset), */
		/* 	m_content_type(other.m_content_type) */
		/* { */
		/* 	CLOG("COPY CONSTRUCTOR"); */
		/* } */

		/* Response& operator=(const Response& other) */
		/* { */
		/* 	CLOG("COPY ASSIGNMENT"); */

		/* 	m_code = other.m_code; */
		/* 	m_content_type = other.m_content_type; */
		/* 	m_charset = other.m_charset; */

		/* 	return *this; */
		/* } */

		/* Response(Response&& other) */
		/* 	: */
		/* 	m_code(other.m_code), */
		/* 	m_charset(other.m_charset), */
		/* 	m_content_type(other.m_content_type) */
		/* { */
		/* 	CLOG("MOVE CONSTRUCTOR"); */
		/* } */

		/* Response& operator=(Response&& other) */
		/* { */
		/* 	CLOG("MOVE ASSIGNMENT"); */

		/* 	m_code = other.m_code; */
		/* 	m_content_type = other.m_content_type; */
		/* 	m_charset = other.m_charset; */

		/* 	return *this; */
		/* } */

	private:
		http::Code m_code = http::Code::OK;
		http::ContentType m_content_type = http::ContentType::TEXT_HTML;
		http::Charset m_charset = http::Charset::UTF_8;
		std::string m_content;
		/* char m_content[BODY_BUFFER_SIZE]; */
	};
}

#endif // RESPONSE_H_
