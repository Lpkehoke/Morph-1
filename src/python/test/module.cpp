#include "python/test/internalsinspector.h"

#include "python/module.h"
#include "python/class.h"
#include "python/trampoline.h"

#include <memory>
#include <string>

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

    bool take_nocopyable_shared_ptr(std::shared_ptr<nocopyable> value)
    {
        if (!value || (value->foo() != "foo"))
        {
            return false;
        }

        value->set_foo("bar");
        return true;
    }
};

class abstract_class : public py::trampoline<abstract_class>
{
  public:
    std::string say_hello()
    {
        return "hello";
    }

    std::string call_say_abstract()
    {
        return say_abstract();
    }

    std::string say_abstract()
    {
        auto res = invoke_python_impl("say_abstract");
        return py::loader<std::string>::load(res);
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
        .def("take_nocopyable_ref", &test_parameter_values::take_nocopyable_ref)
        .def("take_nocopyable_shared_ptr", &test_parameter_values::take_nocopyable_shared_ptr);
    
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

    py::class_<abstract_class>(m, "AbstractClass")
        .def(py::init<>())
        .def("say_hello", &abstract_class::say_hello)
        .def_abstract("say_abstract")
        .def("call_say_abstract", &abstract_class::call_say_abstract);
    
    py::class_<internals_inspector>(m, "InternalsInspector")
        .def(py::init<>())
        .def("instances_count", &internals_inspector::instances_count)
        .def("dump_instances", &internals_inspector::dump_instances);
}

} // namespace test
