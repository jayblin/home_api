#ifndef DB_H_
#define DB_H_

#include <filesystem>
#include <string_view>

namespace db
{
	/* void init(std::string_view filepath); */

	/* void open(std::string_view fullpath); */

	/* sqlw::Connection init_connection( */
	/*     std::string_view database_dir, */
	/*     std::string_view database_filename */
	/* ); */

	namespace migration
	{
		int migrate(
			std::filesystem::path database_path,
			std::filesystem::path migrations_dir
		);
	} // namespace migration

	/* sqlw::Connection init_db_connection(); */

	/* std::string generate_select_last_insert(const std::string_view
	 * table_name); */

	/* sqlw::Connection */
	/*     init_db_and_write_status(std::optional<::http::Response>& response);
	 */

	/* bool exec_query( */
	/*     std::optional<::http::Response>& response, */
	/*     const std::string_view sql, */
	/*     std::function<void(sqlw::Statement&)> binder = nullptr, */
	/*     std::function<std::string(sqlw::JsonStringResult&)> extractor =
	 * nullptr */
	/* ); */

	/* template<typename T> */
	/* requires constraint::has_table_name<T> && constraint::has_columns<T> */
	/* void bind_values(sqlw::Statement& stmt, const nlohmann::json& data) */
	/* { */
	/* 	int idx = 0; */

	/* 	std::apply( */
	/* 	    [&idx, &stmt, &data](auto&&... column) */
	/* 	    { */
	/* 		    ((stmt.bind( */
	/* 		         ++idx, */
	/* 		         get_value( */
	/* 		             std::get<constraint::column_name_t>(column), */
	/* 		             std::get<sqlw::Type>(column), */
	/* 		             data */
	/* 		         ), */
	/* 		         std::get<sqlw::Type>(column) */
	/* 		     )), */
	/* 		     ...); */
	/* 	    }, */
	/* 	    T::columns */
	/* 	); */
	/* } */

	/* bool transact( */
	/*     std::optional<::http::Response>& response, */
	/*     const std::string sql, */
	/*     std::function<void(sqlw::Statement& stmt)> binder, */
	/*     std::function<std::string(sqlw::JsonStringResult&)> emplacer */
	/* ); */

	/* template<size_t Size> */
	/* std::string concat_queries(std::array<std::string, Size> queries) */
	/* { */
	/* 	std::stringstream ss; */

	/* 	for (const auto& q : queries) */
	/* 	{ */
	/* 		ss << q << ";"; */
	/* 	} */

	/* 	return ss.str(); */
	/* } */

	/* template<typename T> */
	/* requires constraint::has_table_name<T> && constraint::has_columns<T> */
	/* std::string generate_insert_clause() */
	/* { */
	/* 	std::stringstream ss; */
	/* 	ss << "INSERT INTO " << T::table_name << " ("; */

	/* 	std::apply( */
	/* 	    [&ss](auto&&... column) */
	/* 	    { */
	/* 		    size_t i = 0; */
	/* 		    constexpr auto cols_number = */
	/* 		        std::tuple_size<typeof(T::columns)>(); */

	/* 		    (((ss << std::get<constraint::column_name_t>(column)), */
	/* 		      (++i < cols_number && ss << ',')), */
	/* 		     ...); */
	/* 	    }, */
	/* 	    T::columns */
	/* 	); */

	/* 	ss << ") VALUES ("; */

	/* 	std::apply( */
	/* 	    [&ss](auto&&... column) */
	/* 	    { */
	/* 		    size_t i = 0; */
	/* 		    constexpr auto cols_number = */
	/* 		        std::tuple_size<typeof(T::columns)>(); */

	/* 		    ((column, (ss << '?'), (++i < cols_number && ss << ',')), ...);
	 */
	/* 	    }, */
	/* 	    T::columns */
	/* 	); */

	/* 	ss << ')'; */

	/* 	return ss.str(); */
	/* } */

	/* template<size_t Size> */
	/* static constexpr std::string_view */
	/*     part_name(std::array<constraint::Constraint, Size>& constraints) */
	/* { */
	/* 	for (const auto c : constraints) */
	/* 	{ */
	/* 		switch (c) */
	/* 		{ */
	/* 			case constraint::Constraint::NOT_EMPTY: */
	/* 				return "NOT_EMPTY"; */
	/* 			case constraint::Constraint::UNIQUE: */
	/* 				return "UNIQUE"; */
	/* 			case constraint::Constraint::PRIMARY: */
	/* 				return "PRIMARY"; */
	/* 			case constraint::Constraint::KEY: */
	/* 				return "KEY"; */
	/* 			case constraint::Constraint::AUTOINCREMENT: */
	/* 				return "AUTOINCREMENT"; */
	/* 			case constraint::Constraint::NOT_NULL: */
	/* 				return "NOT_NULL"; */
	/* 		} */
	/* 	} */
	/* } */

	/* static constexpr std::string_view part_name(sqlw::Type type) */
	/* { */
	/* 	switch (type) */
	/* 	{ */
	/* 		case sqlw::Type::SQL_BLOB: */
	/* 			return "BLOB"; */
	/* 		case sqlw::Type::SQL_DOUBLE: */
	/* 			return "REAL"; */
	/* 		case sqlw::Type::SQL_INT: */
	/* 			return "INTEGER"; */
	/* 		case sqlw::Type::SQL_NULL: */
	/* 			return "NULL"; */
	/* 		case sqlw::Type::SQL_TEXT: */
	/* 			return "TEXT"; */
	/* 	} */
	/* } */

	/* static constexpr std::string_view part_name(constraint::column_name_t
	 * name) */
	/* { */
	/* 	return name; */
	/* } */

	/* static constexpr std::string_view part_name(constraint::Constraint c) */
	/* { */
	/* 	switch (c) */
	/* 	{ */
	/* 		case constraint::Constraint::UNIQUE: */
	/* 			return "UNIQUE"; */
	/* 		case constraint::Constraint::PRIMARY: */
	/* 			return "PRIMARY"; */
	/* 		case constraint::Constraint::KEY: */
	/* 			return "KEY"; */
	/* 		case constraint::Constraint::AUTOINCREMENT: */
	/* 			return "AUTOINCREMENT"; */
	/* 		case constraint::Constraint::NOT_NULL: */
	/* 			return "NOT_NULL"; */
	/* 	} */
	/* } */

	/* template<typename T> */
	/* requires constraint::has_table_name<T> && constraint::has_columns<T> */
	/* static std::string bob() */
	/* { */
	/* 	std::stringstream ss; */
	/* 	ss << "CREATE TABLE IF NOT EXISTS " << T::table_name << '('; */

	/* 	std::apply( */
	/* 	    [&ss](auto&&... column) */
	/* 	    { */
	/* 		    ((std::apply( */
	/* 		         [&ss](auto&&... part) */
	/* 		         { */
	/* 			         ((ss << part_name(part)), ...); */
	/* 		         }, */
	/* 		         column */
	/* 		     )), */
	/* 		     ...); */
	/* 	    }, */
	/* 	    T::columns */
	/* 	); */

	/* 	ss << ')'; */

	/* 	return ss.str(); */
	/* } */

	/* template<typename T> */
	/* requires constraint::has_table_name<T> && constraint::has_columns<T> */
	/* void create_table_from_constraint(sqlw::Connection& con) */
	/* { */
	/* 	sqlw::Statement stmt {&con}; */
	/* 	stmt(bob<T>()); */
	/* } */

} // namespace db

/* struct Query */
/* { */
/* 	enum class Type */
/* 	{ */
/* 		SELECT, */
/* 		INSERT, */
/* 		UPDATE, */
/* 		DELETE, */
/* 	}; */

/* 	struct Argument */
/* 	{ */
/* 		std::string_view value; */
/* 		sqlw::Type type; */
/* 	}; */

/* 	std::string_view sql; */
/* 	std::vector<Argument> arguments; */
/* 	Type type; */
/* }; */

/* Query generate_paginated_select_query_and_arguments( */
/*     const tables::Definition& definition, */
/*     const http::Request& request */
/* ) */
/* { */
/* 	std::stringstream ss; */
/* 	ss << "SELECT e0.* FROM " << definition.table_name << " e0"; */

/* 	std::vector<Query::Argument> query_args; */

/* 	if (request.query.length() > 0) */
/* 	{ */
/* 		http::Parsor parsor {request.query}; */
/* 		http::QueryParser parser; */
/* 		const auto q = parser.parse(parsor); */

/* 		if (q.contains("id")) */
/* 		{ */
/* 			const auto& id = q.at("id"); */

/* 			if (id.values.size() > 0) */
/* 			{ */
/* 				ss << " WHERE e0.id IN ("; */

/* 				size_t i = 0; */
/* 				for (const auto& v : id.values) */
/* 				{ */
/* 					ss << "?"; */

/* 					if (i < (id.values.size() - 1)) */
/* 					{ */
/* 						ss << ","; */
/* 					} */

/* 					query_args.push_back({v, sqlw::Type::SQL_INT}); */
/* 					i++; */
/* 				} */

/* 				ss << ")"; */
/* 			} */
/* 			else if (!id.value.empty()) */
/* 			{ */
/* 				ss << " WHERE e0.id = ?"; */
/* 				query_args.push_back({id.value, sqlw::Type::SQL_INT}); */
/* 			} */
/* 		} */

/* 		if (q.contains("limit") && !q.at("limit").value.empty()) */
/* 		{ */
/* 			ss << " LIMIT ?"; */

/* 			query_args.push_back({q.at("limit").value, sqlw::Type::SQL_INT});
 */

/* 			if (q.contains("page") && !q.at("page").value.empty()) */
/* 			{ */
/* 				ss << " OFFSET ?"; */
/* 				query_args.push_back( */
/* 				    {std::to_string( */
/* 				         std::min(50, std::stoi(q.at("limit").value)) */
/* 				         * (std::stoi(q.at("page").value) - 1) */
/* 				     ), */
/* 				     sqlw::Type::SQL_INT} */
/* 				); */
/* 			} */
/* 		} */
/* 		else */
/* 		{ */
/* 			ss << " LIMIT 50"; */
/* 		} */
/* 	} */

/* 	/1* return {ss.str(), query_args, Query::Type::SELECT}; *1/ */
/* 	return {std::string {"aaaaaa"}, query_args, Query::Type::SELECT}; */
/* } */

#endif // DB_H_
