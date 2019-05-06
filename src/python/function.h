#pragma once

#include "python/pythonapi.h"
#include "python/typecaster.h"

#include <tuple>
#include <type_traits>
#include <utility>


namespace py
{
namespace detail
{


template <typename Return, typename... Args>
struct fn_signature_t {};


template <typename T>
struct fn_signature_from_lambda_impl_t {};


template <typename Return, typename Class, typename... Args>
struct fn_signature_from_lambda_impl_t<Return (Class::*)(Args...)>
{
    using type = fn_signature_t<Return, Args...>;
};


template <typename Return, typename Class, typename... Args>
struct fn_signature_from_lambda_impl_t<Return (Class::*)(Args...) const>
{
    using type = fn_signature_t<Return, Args...>;
};


template <typename Fn>
struct fn_signature_from_lambda_t
{
    using operator_t = decltype(&Fn::operator());
    using type = typename fn_signature_from_lambda_impl_t<operator_t>::type;
};


/**
 *  Converts python arguments tuple to cpp values and
 *  passes them to the function. 
 */
template <typename Fn, typename Return, typename... Args>
struct function_invocation
{
    template <bool is_none>
    struct return_is_none_tag {};

    using arg_index_t = std::index_sequence_for<Args...>;
    using return_is_none_t = typename std::conditional<
                                std::is_void<Return>::value,
                                return_is_none_tag<true>,
                                return_is_none_tag<false>>::type;

    static handle invoke(Fn& fn, tuple py_args)
    {
        return invoke(fn, std::move(py_args), return_is_none_t {});
    }

    static handle invoke(Fn& fn, tuple py_args, return_is_none_tag<false>)
    {
        try
        {
            auto cpp_args = load_arguments(py_args, arg_index_t {});

            Return res = std::apply(fn, std::move(cpp_args));

            return type_caster<Return>::cast(std::move(res));
        }
        catch (const std::exception& ex)
        {
            PyErr_SetString(PyExc_RuntimeError, ex.what());
            return nullptr;
        }
    }

    static handle invoke(Fn& fn, tuple py_args, return_is_none_tag<true>)
    {
        try
        {
            auto cpp_args = load_arguments(py_args, arg_index_t {});

            std::apply(fn, std::move(cpp_args));

            Py_XINCREF(Py_None);
            return Py_None;
        }
        catch (const std::exception& ex)
        {
            PyErr_SetString(PyExc_RuntimeError, ex.what());
            return nullptr;
        }
    }

    template <std::size_t... idx>
    static std::tuple<Args...> load_arguments(tuple py_args, std::index_sequence<idx...>)
    {
        return std::tuple<Args...>(type_caster<Args>::load(PyTuple_GetItem(py_args.ptr(), idx))...);
    }
};


/**
 *  function_record is held in capsule with python function objects.
 *  It is used to invoke actual cpp function during python function call.
 */
struct function_record
{
    using capture_t = std::aligned_storage<sizeof(void*)>;
    using impl_t = handle (*)(void* capture, tuple args);
    using dtor_t = void (*)(void*);

    ~function_record()
    {
        if (m_dtor)
        {
            (*m_dtor)(reinterpret_cast<void*>(&m_capture));
        }
    }

    // Capture destructor.
    dtor_t      m_dtor;

    // Wrapped cpp function (see cpp_function for details).
    capture_t   m_capture;

    // Python method definition.
    PyMethodDef m_def;
};

class cpp_function
{
  public:
    /**
     *  Bind non-const class function.
     */
    template <typename Return, typename Class, typename... Args>
    cpp_function(const char* name, object scope, Return (Class::*fn)(Args...))
    {
        initialize(
            name,
            std::move(scope),
            fn_signature_t<Return, Class&, Args...> {},
            [fn](Class& cls, Args&&... args)
            {
                return (cls.*fn)(std::forward<Args>(args)...);
            });
    }

    /**
     *  Bind lambda object.
     */
    template <typename Fn>
    cpp_function(const char* name, object scope, Fn&& fn)
    {
        initialize(
            name,
            std::move(scope),
            typename fn_signature_from_lambda_t<Fn>::type {},
            std::forward<Fn>(fn));
    }

    /**
     *  Bind free function.
     */
    template <typename Return, typename... Args>
    cpp_function(const char* name, object scope, Return (*fn)(Args...))
    {
        initialize(
            name,
            std::move(scope),
            fn_signature_t<Return, Args...> {},
            [fn](Args&&... args)
            {
                return (*fn)(std::forward<Args>(args)...);
            });
    }

  private:
    template <typename Fn>
    static constexpr bool can_embed = sizeof(Fn) <= sizeof(function_record::capture_t);

    template <typename Fn, typename Return, typename... Args>
    void initialize(const char* name, object scope, fn_signature_t<Return, Args...>, Fn&& fn)
    {
        function_record* fn_rec = new function_record();
        
        capsule fn_rec_c(
            fn_rec,
            [](function_record* fn_rec)
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

        object meth = PyCFunction_New(&def, fn_rec_c.release().ptr());

        if (PyModule_Check(scope.ptr()))
        {
            PyModule_AddObject(scope.ptr(), name, meth.release().ptr());
        }
        else
        {
            object inst_meth = PyInstanceMethod_New(meth.release().ptr());
            scope.setattr(name, inst_meth.release());
        }
    }

    template <typename Fn, typename Return, typename... Args>
    static PyObject* dispatch(PyObject* fn_rec_c, PyObject* py_args)
    {
        auto fn_rec = reinterpret_cast<function_record*>(PyCapsule_GetPointer(fn_rec_c, nullptr));

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

        return function_invocation<Fn, Return, Args...>::invoke(*fn_ptr, py_args).ptr();
    }
};

} // namespace detail
} // namespace py
