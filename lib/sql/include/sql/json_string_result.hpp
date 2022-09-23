#include <sstream>
#include <string>

/**
 * Object to pass to `sqlite3_exec` function.
 */
class JsonStringResult
{
   public:
	/**
	 * Callback to pass to `sqlite3_exec` function.
	 */
	static int callback(void* obj, int argc, char** argv, char** col_name);

	/**
	 * After `sqlite3_exec` finishes this function can be called to return
	 * query result as JSON array.
	 */
	auto get_array_result() -> std::string;

   private:
	std::stringstream m_stream;
};
