#pragma once

#include "scene/renderer.h"

#include <memory>


namespace vk
{

class RendererFactory
{
  public:
    static std::shared_ptr<scene::Renderer> create();
};

} // namespace vk
