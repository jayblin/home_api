#include "routes/ingredients.hpp"

#include "utils.hpp"

#include <fstream>

Response routes::ingredients::list(http::Request& request)
{
	using namespace rapidjson;
	using namespace http;

	const auto file_path = std::string_view{"./_db/ingredients_1.json"};

	check_file_and_write_on_empty(file_path, "[]");

	auto file_stream = std::ifstream(file_path.data());

	if (file_stream.bad())
	{
		file_stream.close();
		return {Code::INTERNAL_SERVER_ERROR, "Ошибка чтения файла"};
	}

	auto str_stream = std::stringstream{};
	str_stream << file_stream.rdbuf();

	file_stream.close();

	return {
		Code::OK,
		str_stream.str(),
		ContentType::APP_JSON
	};
}

Response routes::ingredients::post(http::Request& request)
{
	using namespace rapidjson;
	using namespace http;

	return {Code::OK, "[]", ContentType::APP_JSON};

	auto request_body = Document{};

	request_body.Parse(request.body.c_str());

	if (
		request_body.IsNull()
		|| !request_body.HasMember("title")
	)
	{
		return {
			Code::BAD_REQUEST,
			"{\"errors\": [\"Не хватает параметров\"]}",
			ContentType::APP_JSON,
		};
	}

	const auto file_path = std::string_view{"./_db/ingredients_1.json"};

	check_file_and_write_on_empty(file_path, "[]");
	
	auto file_stream = std::fstream{
		file_path.data(),
		std::ofstream::in
		| std::ofstream::out
	};

	if (file_stream.bad())
	{
		file_stream.close();
		return {
			Code::INTERNAL_SERVER_ERROR,
			"{\"errors\": [\"Ошибка чтения файла\"]}",
			ContentType::APP_JSON,
		};
	}

	auto str_stream = std::stringstream{};
	str_stream << file_stream.rdbuf();

	if (str_stream.view().size() < 2)
	{
		str_stream.clear();
	}

	auto d = rapidjson::Document{};

	d.Parse(str_stream.view().data());

	auto n = d.Size();

	auto ingredient = rapidjson::Value{rapidjson::kObjectType};
	ingredient.AddMember("id", n, d.GetAllocator());
	ingredient.AddMember("title", request_body["title"], d.GetAllocator());

	d.PushBack(ingredient, d.GetAllocator());

	file_stream.seekp(0);
	file_stream << json_doc_to_string(d);

	file_stream.close();

	return {Code::OK, "[]", ContentType::APP_JSON};
}
