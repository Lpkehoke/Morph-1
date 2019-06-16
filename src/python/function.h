#pragma once

#include "python/cast.h"
#include "python/pythonapi.h"

#include <tuple>
#include <type_traits>
#include <utility>


namespace py
{

template <typename... Args>
struct Init {};

namespace detail
{

template <typename Return, typename... Args>
struct FnSignatureT {};


template <typename T>
struct FnSignatureFromLambdaImpl {};


template <typename Return, typename Class, typename... Args>
struct FnSignatureFromLambdaImpl<Return (Class::*)(Args...)>
{
    using Type = FnSignatureT<Return, Args...>;
};


template <typename Return, typename Class, typename... Args>
struct FnSignatureFromLambdaImpl<Return (Class::*)(Args...) const>
{
    using Type = FnSignatureT<Return, Args...>;
};


template <typename Fn>
struct FnSignatureFromLambdaT
{
    using OperatorType = decltype(&Fn::operator());
    using Type = typename FnSignatureFromLambdaImpl<OperatorType>::type;
};


template <typename... Args, std::size_t... idx>
std::tuple<Args...> python_to_cpp_tuple(Tuple py_tuple, std::index_sequence<idx...>)
{
    return std::tuple<Args...>(Loader<Args>::load(py_tuple[idx])...);
}


template <typename... Args>
Tuple cpp_to_python_tuple_impl(Args&&... args)
{
    auto py_tuple = PyTuple_Pack(
        sizeof...(Args),
        Caster<Args>::cast(
            std::forward<Args>(args),
            return_value_policy::copy).ptr()...); // TODO: use auto return value policy
    
    return py_tuple;
}


template <typename... Args>
Tuple cpp_to_python_tuple(std::tuple<Args...>&& cpp_tuple)
{
    return std::apply(cpp_to_python_tuple_impl<Args...>, std::forward<std::tuple<Args...>>(cpp_tuple));
}


/*
 *  Converts python arguments tuple to cpp values and
 *  passes them to the function.
 */
template <typename Fn, typename Return, typename... Args>
struct FunctionInvocation
{
    template <bool is_none>
    struct ReturnIsNoneTag {};

    using ArgIndex = std::index_sequence_for<Args...>;
    using ReturnIsNone = ReturnIsNoneTag<std::is_void<Return>::value>;

    static Handle invoke(
        Fn&                 fn,
        Tuple               py_args,
        return_value_policy policy)
    {
        try
        {
            return invoke(fn, std::move(py_args), policy, ReturnIsNone {});
        }
        catch (const ErrorAlreadySet&)
        {
            return nullptr;
        }
        catch (const std::exception& ex)
        {
            PyErr_SetString(PyExc_RuntimeError, ex.what());
            return nullptr;
        }
    }

    static Handle invoke(
        Fn&                 fn, 
        Tuple               py_args,
        return_value_policy policy,
        ReturnIsNoneTag<false>)
    {
        auto cpp_args = python_to_cpp_tuple<Args...>(py_args, ArgIndex {});

        Return res = std::apply(fn, std::move(cpp_args));

        return cast(std::forward<Return>(res), policy);
    }

    static Handle invoke(
        Fn&                 fn, 
        Tuple               py_args,
        return_value_policy policy,
        ReturnIsNoneTag<true>)
    {
        auto cpp_args = python_to_cpp_tuple<Args...>(py_args, ArgIndex {});

        std::apply(fn, std::move(cpp_args));

        Py_XINCREF(Py_None);
        return Py_None;
    }
};


/**
 *  function_record is held in capsule with python function objects.
 *  It is used to invoke actual cpp function during python function call.
 */
struct FunctionRecord
{
    using Capture = std::aligned_storage<sizeof(void*)>;
    using Impl = Handle (*)(void* capture, Tuple args);
    using Dtor = void (*)(void*);

    ~FunctionRecord()
    {
        if (m_dtor)
        {
            (*m_dtor)(reinterpret_cast<void*>(&m_capture));
        }
    }

    return_value_policy m_policy;

    // Capture destructor.
    Dtor                m_dtor;

    // Wrapped cpp function (see cpp_function for details).
    Capture             m_capture;

    // Python method definition.
    PyMethodDef         m_def;
};

class CppFunction
{
  public:
    /**
     *  Bind non-const class function.
     */
    template <typename Return, typename Class, typename... Args>
    CppFunction(
        const char*         name,
        Object              scope,
        return_value_policy policy,
        Return (Class::*fn)(Args...))
    {
        initialize(
            name,
            std::move(scope),
            policy,
            [fn](Class& cls, Args&&... args) -> Return
            {
                return (cls.*fn)(std::forward<Args>(args)...);
            },
            FnSignatureT<Return, Class&, Args...> {});
    }

    /**
     *  Bind const class function.
     */
    template <typename Return, typename Class, typename... Args>
    CppFunction(
        const char*         name,
        Object              scope,
        return_value_policy policy,
        Return (Class::* fn)(Args...) const)
    {
        initialize(
            name,
            std::move(scope),
            policy,
            [fn](const Class& cls, Args&&... args) -> Return
            {
                return (cls.*fn)(std::forward<Args>(args)...);
            },
            FnSignatureT<Return, const Class&, Args...> {});
    }

    /**
     *  Bind lambda object.
     */
    template <typename Fn>
    CppFunction(
        const char*         name,
        Object              scope,
        return_value_policy policy,
        Fn&&                fn)
    {
        initialize(
            name,
            std::move(scope),
            policy,
            std::forward<Fn>(fn),
            typename FnSignatureFromLambdaT<Fn>::type {});
    }

    /**
     *  Bind free function.
     */
    template <typename Return, typename... Args>
    CppFunction(
        const char*         name,
        Object              scope,
        return_value_policy policy,
        Return (*fn)(Args...))
    {
        initialize(
            name,
            std::move(scope),
            policy,
            [fn](Args&&... args)
            {
                return (*fn)(std::forward<Args>(args)...);
            },
            FnSignatureT<Return, Args...> {});
    }

    /**
     *  Bind constructor.
     */
    template <typename Class, typename... Args>
    CppFunction(Init<Class, Args...>, Object scope)
    {
        initialize(
            "__init__",
            std::move(scope),
            return_value_policy::copy,
            [](Class* this_ptr, Args&&... args)
            {
                new (this_ptr) Class(std::forward<Args>(args)...);
            },
            FnSignatureT<void, Class*, Args...> {});
    }

  private:
    template <typename Fn>
    static constexpr bool can_embed = sizeof(Fn) <= sizeof(FunctionRecord::Capture);

    template <typename Fn, typename Return, typename... Args>
    void initialize(
        const char*         name,
        Object              scope,
        return_value_policy policy,
        Fn&&                fn,
        FnSignatureT<Return, Args...>)
    {
        FunctionRecord* fn_rec = new FunctionRecord();
        fn_rec->m_policy = policy;

        Capsule fn_rec_c(
            fn_rec,
            [](FunctionRecord* fn_rec)
            {
                delete fn_rec;
            });

        auto& def = fn_rec->m_def;
        def.ml_name = name;
        def.ml_doc = nullptr;
        def.ml_flags = METH_VARARGS;
        def.ml_meth = &dispatch<Fn, Return, Args...>;

        if (can_embed<Fn>)
        {
            // Capture holds the functor itself.
            auto capture_ptr = reinterpret_cast<Fn*>(&fn_rec->m_capture);
            new (capture_ptr) Fn(std::forward<Fn>(fn));

            fn_rec->m_dtor = [](void* capture_ptr)
            {
                delete reinterpret_cast<Fn*>(capture_ptr);
            };
        }
        else
        {
            // Capture holds a pointer to the functor.
            auto capture_ptr = reinterpret_cast<Fn**>(&fn_rec->m_capture);
            *capture_ptr = new Fn(std::forward<Fn>(fn));

            fn_rec->m_dtor = [](void* capture_ptr)
            {
                auto fn_ptr = reinterpret_cast<Fn**>(capture_ptr);
                delete *fn_ptr;
            };
        }

        Object meth = PyCFunction_New(&def, fn_rec_c.release().ptr());

        if (PyModule_Check(scope.ptr()))
        {
            PyModule_AddObject(scope.ptr(), name, meth.release().ptr());
        }
        else
        {
            Object inst_meth = PyInstanceMethod_New(meth.release().ptr());
            PyDict_SetItemString(scope.ptr(), name, inst_meth.ptr());
        }
    }

    template <typename Fn, typename Return, typename... Args>
    static PyObject* dispatch(PyObject* fn_rec_c, PyObject* py_args)
    {
        auto fn_rec = reinterpret_cast<FunctionRecord*>(PyCapsule_GetPointer(fn_rec_c, nullptr));

        if (sizeof...(Args) != PyTuple_Size(py_args))
        {
            PyErr_Format(
                PyExc_TypeError,
                "Function requires %d positional arguments, %d provided.",
                sizeof...(Args),
                PyTuple_Size(py_args));
            return nullptr;
        }

        Fn* fn_ptr;
        if (can_embed<Fn>)
        {
            fn_ptr = reinterpret_cast<Fn*>(&fn_rec->m_capture);
        }
        else
        {
            fn_ptr = *reinterpret_cast<Fn**>(&fn_rec->m_capture);
        }

        return FunctionInvocation<Fn, Return, Args...>::invoke(
            *fn_ptr,
            py_args,
            fn_rec->m_policy).ptr();
    }
};

} // namespace detail

template <typename... Args>
Handle Handle::operator()(Args&&... args)
{
    auto py_args = detail::cpp_to_python_tuple(std::tuple {std::forward<Args>(args)...});
    return PyObject_Call(m_ptr, py_args.ptr(), nullptr);
}

} // namespace py
