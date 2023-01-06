#include "tables/definitions.hpp"

const std::vector<tables::Definition>& tables::definitions()
{
	using C = tables::Constraint;

	static const std::vector<tables::Definition> _definitions {
		{
			"food",
			{
				{
					"title",
					"Название продукта",
					sql::Type::SQL_TEXT,
					{ C::NOT_EMPTY, C::UNIQUE }
				},
				{"calories", "Кол-во калорий", sql::Type::SQL_DOUBLE},
				{"proteins", "Кол-во белка", sql::Type::SQL_DOUBLE},
				{"carbohydrates", "Кол-во углеводов", sql::Type::SQL_DOUBLE},
				{"fats", "Кол-во жиров", sql::Type::SQL_DOUBLE}
			}
		},
	};

	return _definitions;
}
