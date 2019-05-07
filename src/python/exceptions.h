#include <exception>

class error_already_set : public std::exception
{
  public:
    error_already_set()  = default;
    ~error_already_set() = default;
};
