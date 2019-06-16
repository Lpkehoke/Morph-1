#include "python/test/internalsinspector.h"

#include "python/module.h"
#include "python/class.h"
#include "python/trampoline.h"

#include <memory>
#include <string>

namespace test
{

struct DummyA
{
    std::string a;

    DummyA()
      : a("A")
    {}

    std::string say_a()
    {
        return a;
    }
};

struct DummyB
{
    std::string b;

    DummyB()
      : b("B")
    {}

    std::string say_b()
    {
        return b;
    }
};

struct DummyC
{
    std::string c;

    DummyC()
      : c("C")
    {}

    std::string say_c()
    {
        return c;
    }
};

struct DummyD
{
    std::string d;

    DummyD()
      : d("D")
    {}

    std::string say_d()
    {
        return d;
    }
};

class Nocopyable
{
  public:
    Nocopyable()
      : m_foo("foo")
    {}

    Nocopyable(const Nocopyable& other) = delete;
    Nocopyable(Nocopyable&& other) = default;

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

class TestReturnValues
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

    Nocopyable get_nocopyable()
    {
        return Nocopyable();
    }

    Nocopyable& get_nocopyable_ref()
    {
        return m_nocopyable;
    }

  private:
    Nocopyable  m_nocopyable;
};

class TestParameterValues
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

    bool take_nocopyable_ref(Nocopyable& value)
    {
        if (value.foo() != "foo")
        {
            return false;
        }
        
        value.set_foo("bar");
        return true;
    }

    bool take_nocopyable_shared_ptr(std::shared_ptr<Nocopyable> value)
    {
        if (!value || (value->foo() != "foo"))
        {
            return false;
        }

        value->set_foo("bar");
        return true;
    }
};

class AbstractClass : public py::Trampoline<AbstractClass>
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
        return py::Loader<std::string>::load(res);
    }
};

MORPH_PYTHON_MODULE(_test, m, Morph Python test module)
{
    py::ExposeClass<TestReturnValues>(m, "TestReturnValues")
        .def(py::Init<> {})
        .def("get_one_int", &TestReturnValues::get_one_int)
        .def("get_hello_string", &TestReturnValues::get_hello_string)
        .def(
            "get_nocopyable",
            &TestReturnValues::get_nocopyable,
            py::return_value_policy::move)
        .def(
            "get_nocopyable_ref",
            &TestReturnValues::get_nocopyable_ref,
            py::return_value_policy::reference);

    py::ExposeClass<TestParameterValues>(m, "TestParameterValues")
        .def(py::Init<> {})
        .def("take_one_int", &TestParameterValues::take_one_int)
        .def("take_hello_string", &TestParameterValues::take_hello_string)
        .def("take_nocopyable_ref", &TestParameterValues::take_nocopyable_ref)
        .def("take_nocopyable_shared_ptr", &TestParameterValues::take_nocopyable_shared_ptr);
    
    py::ExposeClass<Nocopyable>(m, "Nocopyable")
        .def(py::Init<>{})
        .def("foo", &Nocopyable::foo)
        .def("set_foo", &Nocopyable::set_foo);

    py::ExposeClass<DummyA>(m, "DummyA")
        .def("say_a", &DummyA::say_a)
        .def(py::Init<>());

    py::ExposeClass<DummyB>(m, "DummyB")
        .def("say_b", &DummyB::say_b)
        .def(py::Init<>());

    py::ExposeClass<DummyC>(m, "DummyC")
        .def("say_c", &DummyC::say_c)
        .def(py::Init<>());

    py::ExposeClass<DummyD>(m, "DummyD")
        .def("say_d", &DummyD::say_d)
        .def(py::Init<>());

    py::ExposeClass<AbstractClass>(m, "AbstractClass")
        .def(py::Init<>())
        .def("say_hello", &AbstractClass::say_hello)
        .def_abstract("say_abstract")
        .def("call_say_abstract", &AbstractClass::call_say_abstract);
    
    py::ExposeClass<InternalsInspector>(m, "InternalsInspector")
        .def(py::Init<>())
        .def("instances_count", &InternalsInspector::instances_count)
        .def("dump_instances", &InternalsInspector::dump_instances);
}

} // namespace test
