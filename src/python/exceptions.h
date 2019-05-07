#include <exception>

namespace py
{

class error_already_set : public std::exception
{
};

} // namespace py
