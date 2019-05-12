#include "python/module.h"
#include "python/class.h"

namespace test
{

class nocopyable
{
  public:
    nocopyable() = default;
    nocopyable(const nocopyable& other) = delete;
    nocopyable(nocopyable&& other) = default;
};

class test_return_values
{
  public:
    int get_one_int()
    {
        return 1; 
    }

    std::string get_hello_string()
    {
        return "hello";
    }

    nocopyable get_nocopyable()
    {
        return nocopyable();
    }

    nocopyable& get_nocopyable_ref()
    {
        return m_nocopyable;
    }

  private:
    nocopyable  m_nocopyable;
};

MORPH_PYTHON_MODULE(_test, m, Morph Python test module)
{
    py::class_<test_return_values>(m, "TestReturnValues")
        .def(py::init<> {})
        .def("get_one_int", &test_return_values::get_one_int)
        .def("get_hello_string", &test_return_values::get_hello_string)
        .def(
            "get_nocopyable",
            &test_return_values::get_nocopyable,
            py::return_value_policy::move)
        .def(
            "get_nocopyable_ref",
            &test_return_values::get_nocopyable_ref,
            py::return_value_policy::reference);
    
    py::class_<nocopyable>(m, "Nocopyable");
}

} // namespace test
