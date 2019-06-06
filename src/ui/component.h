#pragma once

#include "ui/uitypes.h"

namespace ui
{

class component
{
  public:
    struct element_init_info
    {
        component_factory_ptr_t factory;
        
    };
};

class component_factory
{
  public:
    virtual component_ptr_t create() const = 0;
};

} // namespace ui
