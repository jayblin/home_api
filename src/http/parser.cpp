#include "http/parser.hpp"
#include <charconv>
#include <iomanip>
#include <iostream>
#include <system_error>

/* template<size_t NamesSize, size_t DelimSize> */
/* std::array<std::string_view, NamesSize> http::Parser::parse( */
/*     http::Parsor& parsor, */
/*     std::string_view start_str, */
/*     std::array<std::string_view, DelimSize> value_delimiters, */
/*     std::string_view declaration_delimiter, */
/*     std::array<std::string_view, NamesSize> names */
/* ) */
/* { */
/* 	/1* auto iters = 0; *1/ */
/* 	std::array<std::string_view, NamesSize> result; */

/* 	const auto npos = std::string_view::npos; */
/* 	const auto& view = parsor.view(); */
/* 	const auto start_pos = view.find(start_str); */

/* 	if (start_pos == npos) */
/* 	{ */
/* 		m_state = Parser::State::END; */
/* 		return result; */
/* 	} */

/* 	size_t i = 0; */
/* 	for (const auto name : names) */
/* 	{ */
/* 		const auto entry_start = */
/* 		    view.find(name, start_pos + start_str.length()); */

/* 		if (entry_start == npos) */
/* 		{ */
/* 			continue; */
/* 		} */

/* 		const auto assignment_pos = */
/* 		    view.find(declaration_delimiter, entry_start); */

/* 		if (assignment_pos == npos) */
/* 		{ */
/* 			continue; */
/* 		} */

/* 		auto entry_end = npos; */

/* 		for (const auto& delim : value_delimiters) */
/* 		{ */
/* 			entry_end = view.find(delim, assignment_pos); */

/* 			if (entry_end != npos) */
/* 			{ */
/* 				break; */
/* 			} */
/* 		} */

/* 		if (entry_end == npos) */
/* 		{ */
/* 			continue; */
/* 		} */

/* 		result[i++] = std::string_view {view, assignment_pos, 3}; */
/* 	} */

/* 	return result; */
/* } */
