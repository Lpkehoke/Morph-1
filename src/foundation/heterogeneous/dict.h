#pragma once

#include "foundation/immutable/map.h"
#include "foundation/heterogeneous/box.h"

#include <string>

namespace foundation
{
namespace heterogeneous
{

using dict = immutable::map<std::string, box>;

} // namespace heterogeneous
} // namespace foundation
