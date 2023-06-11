#ifndef HA_API_H_
#define HA_API_H_

#include "sqlw/concepts.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include <array>
#include <concepts>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace api
{
	using is_required_t = bool;
	using column_name_t = std::string_view;

	// clang-format off
	template<typename T>
	concept is_value_extractor = requires(T t)
	{
		{ T(std::string_view{}) };
		{ t.exists(column_name_t{}) } -> std::same_as<bool>;
		{ t.is_null(column_name_t{}) } -> std::same_as<bool>;
		{ t.value(column_name_t{}) } -> std::same_as<std::string>;
		{ t.errors() } -> std::same_as<std::vector<std::string>&>;
	};

	// clang-format on

	constexpr auto get_foods_definition()
	{
		return std::array<
		    std::tuple<api::column_name_t, sqlw::Type, api::is_required_t>,
		    5> {
		    {{"title", sqlw::Type::SQL_TEXT, true},
		     {"calories", sqlw::Type::SQL_DOUBLE, false},
		     {"proteins", sqlw::Type::SQL_DOUBLE, false},
		     {"carbohydrates", sqlw::Type::SQL_DOUBLE, false},
		     {"fats", sqlw::Type::SQL_DOUBLE, false}}
        };
	}

	template<typename T, size_t Size>
	requires api::is_value_extractor<T>
	std::vector<std::string> insert(
	    std::string_view table_name,
	    std::array<
	        std::tuple<api::column_name_t, sqlw::Type, api::is_required_t>,
	        Size> columns,
	    T extractor,
	    sqlw::Statement& stmt
	)
	{
		std::vector<std::string> errors;
		std::stringstream ss;
		ss << "INSERT INTO \"" << table_name << "\" (";

		size_t i = 0;
		for (const auto& column : columns)
		{
			ss << std::get<api::column_name_t>(column);

			if (++i < Size)
			{
				ss << ',';
			}
		}

		ss << ") VALUES (";

		i = 0;
		for (const auto& column : columns)
		{
			ss << '?';

			if (++i < Size)
			{
				ss << ',';
			}
		}

		ss << ')';

		stmt.prepare(ss.str());

		size_t arg_idx = 0;
		for (const auto& column : columns)
		{
			arg_idx++;

			std::string_view val;

			if (!extractor.exists(std::get<api::column_name_t>(column))
			    || extractor.is_null(std::get<api::column_name_t>(column)))
			{
				if (std::get<api::is_required_t>(column))
				{
					errors.push_back(
					    std::string {std::get<api::column_name_t>(column)}
					    + " is required"
					);
				}
				else
				{
					switch (std::get<sqlw::Type>(column))
					{
						case sqlw::Type::SQL_DOUBLE:
							val = "0.0";
							break;
						case sqlw::Type::SQL_INT:
							val = "0";
							break;
						case sqlw::Type::SQL_TEXT:
						case sqlw::Type::SQL_BLOB:
						case sqlw::Type::SQL_NULL:
							val = "NULL";
							break;
					}
				}
			}
			else
			{
				switch (std::get<sqlw::Type>(column))
				{
					case sqlw::Type::SQL_DOUBLE:
					case sqlw::Type::SQL_INT:
						val =
						    extractor.value(std::get<api::column_name_t>(column
						    ));
						break;
					case sqlw::Type::SQL_TEXT:
					case sqlw::Type::SQL_BLOB:
						val =
						    extractor.value(std::get<api::column_name_t>(column
						    ));
						break;
					case sqlw::Type::SQL_NULL:
						val = "NULL";
						break;
				}
			}

			if (errors.empty())
			{
				try
				{
					stmt.bind(arg_idx, val, std::get<sqlw::Type>(column));
				}
				catch (const std::invalid_argument)
				{
					std::stringstream iass;
					iass << "Invalid argument for "
					     << std::get<api::column_name_t>(column);
					errors.emplace_back(iass.str());
				}
			}
		}

		return errors;
	}

	template<typename Extractor, typename Retriever>
	requires is_value_extractor<Extractor>
	      && sqlw::can_be_used_by_statement<Retriever>
	std::tuple<Retriever, std::vector<std::string>> create_food(
	    std::string_view values,
	    std::string_view username,
	    sqlw::Connection* db_connection
	)
	{
		std::tuple<Retriever, std::vector<std::string>> result;

		sqlw::Statement stmt {db_connection};

		stmt("SAVEPOINT sp_post_foods");

		if (!sqlw::status::is_ok(stmt.status()))
		{
			std::get<1>(result).emplace_back(sqlw::status::view(stmt.status())
			);

			return result;
		}

		Extractor extractor {values};

		if (extractor.errors().size() > 0)
		{
			std::get<1>(result) = std::move(extractor.errors());

			return result;
		}

		const auto errors =
		    insert("food", get_foods_definition(), extractor, stmt);

		if (!errors.empty())
		{
			stmt("ROLLBACK TO sp_post_foods");

			std::get<1>(result) = std::move(errors);

			return result;
		}

		stmt.exec();

		if (!sqlw::status::is_ok(stmt.status()))
		{
			std::get<1>(result).emplace_back(sqlw::status::view(stmt.status())
			);

			return result;
		}

		stmt.prepare(R"(
		INSERT INTO user_food_log (user_id, food_id)
			SELECT info.user_id, info.food_id
			FROM (
				WITH
				user_info AS (
					SELECT u.id AS user_id
					FROM "user" u
					WHERE u.name = ?
				),
				food_info AS (
					SELECT f.id AS food_id
					FROM food f
					WHERE f.id = last_insert_rowid()
				)
				SELECT * FROM user_info, food_info
			) AS info;
		)");

		stmt.bind(1, username, sqlw::Type::SQL_TEXT);

		if (!sqlw::status::is_ok(stmt.status()))
		{
			std::get<1>(result).emplace_back(sqlw::status::view(stmt.status())
			);

			return result;
		}

		stmt.exec();

		if (!sqlw::status::is_ok(stmt.status()))
		{
			std::get<1>(result).emplace_back(sqlw::status::view(stmt.status())
			);

			return result;
		}

		std::get<Retriever>(result) = stmt.operator()<Retriever>(R"(
		SELECT f.*
		FROM food f
		INNER JOIN user_food_log ufl
			ON ufl.food_id = f.id
		WHERE ufl.id = last_insert_rowid()
	)");

		if (!sqlw::status::is_ok(stmt.status()))
		{
			stmt("ROLLBACK TO sp_post_foods");

			std::get<1>(result).emplace_back(sqlw::status::view(stmt.status())
			);

			return result;
		}

		stmt("RELEASE sp_post_foods");

		if (!sqlw::status::is_ok(stmt.status()))
		{
			std::get<1>(result).emplace_back(sqlw::status::view(stmt.status())
			);

			return result;
		}

		return result;
	}

} // namespace api

#endif // HA_API_H_
