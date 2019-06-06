#include "ui/application.h"

#include "python/module.h"
#include "python/class.h"
#include "python/trampoline.h"


namespace ui
{

MORPH_PYTHON_MODULE(_ui, m, Morph Python ui module)
{
    py::class_<application>(m, "Application")
        .def(py::init<>())
        .def("render", &application::render);
}

} // namespace test
