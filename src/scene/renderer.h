#pragma once

namespace scene
{

class Renderer
{
  public:
    virtual void initialize() = 0;
    virtual ~Renderer() = default;
};

} // namespace scene
