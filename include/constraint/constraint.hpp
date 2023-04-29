#ifndef CONSTRAINT_H_
#define CONSTRAINT_H_

#include <array>
#include <string_view>
#include "sqlw/forward.hpp"
#include <span>
#include <vector>

namespace constraint
{
	typedef std::string_view column_name_t;

	enum class Constraint
	{
		UNIQUE,
		PRIMARY,
		KEY,
		AUTOINCREMENT,
		NOT_NULL,
	};

	struct ErrorAggregator
	{
		void set(const column_name_t& col_name)
		{
			column_name = &col_name;
		}

		void set(const sqlw::Type& col_type)
		{
			column_type = &col_type;
		}

		template<size_t Count>
		void set(const std::array<Constraint, Count>& list)
		{
			constraints = list;
		}

		const column_name_t* column_name {nullptr};
		const sqlw::Type* column_type {nullptr};
		std::span<const Constraint> constraints {};
	};

	// clang-format off
	template<typename T>
	concept has_table_name = requires()
	{
		{ T::table_name } -> std::convertible_to<std::string_view>;
	};
	// clang-format on

	// clang-format off
	template<typename T>
	concept has_columns = requires()
	{
		/* { T::columns } -> std::convertible_to<std::tuple<std::tuple<column_name_t, sqlw::Type>>>; */
		{ T::columns };
	};
	// clang-format on

	struct Food
	{
		using C = Constraint;

		static constexpr std::string_view table_name = "food";

		static constexpr std::tuple<
		    std::tuple<column_name_t, sqlw::Type, std::array<Constraint, 2>>,
		    std::tuple<column_name_t, sqlw::Type>,
		    std::tuple<column_name_t, sqlw::Type>,
		    std::tuple<column_name_t, sqlw::Type>,
		    std::tuple<column_name_t, sqlw::Type>>
		    columns {
		        {"title", sqlw::Type::SQL_TEXT, {C::NOT_NULL, C::UNIQUE}},
		        {"calories", sqlw::Type::SQL_DOUBLE},
		        {"proteins", sqlw::Type::SQL_DOUBLE},
		        {"carbohydrates", sqlw::Type::SQL_DOUBLE},
		        {"fats", sqlw::Type::SQL_DOUBLE}
        };
	};
	static_assert(has_table_name<Food>);
	static_assert(has_columns<Food>);
}

#endif // CONSTRAINT_H_
