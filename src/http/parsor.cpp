#include "http/parsor.hpp"

void http::Parsor::advance(size_t increment)
{
	 if (!is_end())
	 {
		 m_cur_pos += std::min(increment, m_str.length() - m_cur_pos - 1);
	 }
}
