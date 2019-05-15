#include "python/module.h"
#include "python/class.h"

#include <string>
#include <iostream>

namespace test
{

struct dummy_a
{
    std::string a;

    dummy_a()
      : a("A")
    {}

    std::string say_a()
    {
        return a;
    }
};

struct dummy_b
{
    std::string b;

    dummy_b()
      : b("B")
    {}

    std::string say_b()
    {
        return b;
    }
};

struct dummy_c
{
    std::string c;

    dummy_c()
      : c("C")
    {}

    std::string say_c()
    {
        return c;
    }
};

struct dummy_d
{
    std::string d;

    dummy_d()
      : d("D")
    {}

    std::string say_d()
    {
        return d;
    }
};

class nocopyable
{
  public:
    nocopyable()
      : m_foo("foo")
    {}

    nocopyable(const nocopyable& other) = delete;
    nocopyable(nocopyable&& other) = default;

    std::string foo() const
    {
        return m_foo;
    }

    void set_foo(std::string value)
    {
        m_foo = value;
    }
  
  private:
    std::string m_foo;
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

class test_parameter_values
{
  public:
    bool take_one_int(int value)
    {
        return value == 1;
    }

    bool take_hello_string(std::string value)
    {
        return value == "hello";
    }

    bool take_nocopyable_ref(nocopyable& value)
    {
        if (value.foo() != "foo")
        {
            return false;
        }
        
        value.set_foo("bar");
        return true;
    }
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

    py::class_<test_parameter_values>(m, "TestParameterValues")
        .def(py::init<> {})
        .def("take_one_int", &test_parameter_values::take_one_int)
        .def("take_hello_string", &test_parameter_values::take_hello_string)
        .def("take_nocopyable_ref", &test_parameter_values::take_nocopyable_ref);
    
    py::class_<nocopyable>(m, "Nocopyable")
        .def(py::init<>{})
        .def("foo", &nocopyable::foo)
        .def("set_foo", &nocopyable::set_foo);

    py::class_<dummy_a>(m, "DummyA")
        .def("say_a", &dummy_a::say_a)
        .def(py::init<>());

    py::class_<dummy_b>(m, "DummyB")
        .def("say_b", &dummy_b::say_b)
        .def(py::init<>());

    py::class_<dummy_c>(m, "DummyC")
        .def("say_c", &dummy_c::say_c)
        .def(py::init<>());

    py::class_<dummy_d>(m, "DummyD")
        .def("say_d", &dummy_d::say_d)
        .def(py::init<>());
}

} // namespace test
