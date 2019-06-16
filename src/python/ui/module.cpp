#include "ui/application.h"

#include "python/cast.h"
#include "python/class.h"
#include "python/module.h"
#include "python/trampoline.h"

#include <memory>
#include <utility>

namespace ui
{

MORPH_PYTHON_MODULE(_ui, m, Morph Python ui module)
{
    py::ExposeClass<Application>(m, "Application")
        .def(py::Init<>())
        .def("render", &Application::render);
}

} // namespace test
