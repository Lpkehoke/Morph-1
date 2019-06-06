#pragma once

#include <memory>

namespace ui { class component; }
namespace ui { class component_factory; }

namespace ui
{

using component_ptr_t = std::shared_ptr<component>;
using component_factory_ptr_t = std::shared_ptr<component_factory>;

} // namespace ui
