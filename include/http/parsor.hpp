#ifndef HTTP_PARSOR_H_
#define HTTP_PARSOR_H_

#include <string_view>
/* #include <utility> */
/* #include <algorithm> */

namespace http
{
	/*
	 * A thing that holds a string and points to currently parsed character.
	 */
	class Parsor
	{
	public:
		Parsor(std::string_view str) :
			m_str(str),
			m_cur_pos(m_str.length() > 0 ? 0 : -1) {};

		/**
		 * Return currently parsed character.
		 */
		auto cur_char() const -> char { return m_str[m_cur_pos]; };

		/**
		 * Returns index of currently parsed character.
		 */
		auto cur_pos() const -> size_t { return m_cur_pos; }

		/**
		 * Returns view of the string that is being parsed.
		 */
		auto view() const -> const std::string_view& { return m_str; }

		/**
		 * Detects if currently parsed character is the last one.
		 */
		auto is_end() const -> bool { return cur_pos() == view().length() - 1; }

		/**
		 * Advance currently parsed index by `increment` if not at end of
		 * parsed string.
		 */
		auto advance(size_t increment = 1) -> void;

	private:
		const std::string_view m_str;
		size_t m_cur_pos;
	};
} // namespace http

#endif // HTTP_PARSOR_H_
