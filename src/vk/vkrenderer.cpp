#include "vk/vkrenderer.h"

#include <volk.h>

#include <cstdint>
#include <vector>
#include <iostream>

namespace vk
{

class VkRenderer : public scene::Renderer
{
  public:
    virtual void initialize() override;

  private:
    VkInstance m_instance;
};

void VkRenderer::initialize()
{
    const VkResult init_result = volkInitialize();

    if (init_result != VK_SUCCESS)
    {
        // TODO: error handling.
        std::cout << "Could not load Vulkan." << std::endl;
        return;
    }

    VkApplicationInfo application_info 
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,         // structure type
        nullptr,                                    // next
        "Morph",                                    // application name
        VK_MAKE_VERSION( 1, 0, 0 ),                 // application version
        "Morph",                                    // engine name
        VK_MAKE_VERSION( 1, 0, 0 ),                 // engine version
        VK_API_VERSION_1_0                          // api version
    };

    VkInstanceCreateInfo instance_create_info = 
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,     // structure type
        nullptr,                                    // next
        0,                                          // instance create flags
        &application_info,                          // application info
        0,                                          // enabled layer count
        nullptr,                                    // enabled layer names
        0,                                          // enabled extension count
        nullptr                                     // enabled extension names
    };

    const VkResult create_inst_result = vkCreateInstance(
        &instance_create_info,
        nullptr,
        &m_instance);
    
    if (create_inst_result != VK_SUCCESS)
    {
        std::cout << "Could not create Vulkan instance!" << std::endl;
        return;
    }

    volkLoadInstance(m_instance);

    std::uint32_t num_devices = 0;
    VkResult enumerate_devices_result = vkEnumeratePhysicalDevices(
        m_instance,
        &num_devices,
        nullptr);

    if ((enumerate_devices_result != VK_SUCCESS) || (num_devices == 0))
    {
        std::cout << "Error occurred during physical devices enumeration!" << std::endl;
        return;
    }

    std::vector<VkPhysicalDevice> physical_devices(num_devices);
    enumerate_devices_result = vkEnumeratePhysicalDevices(
        m_instance,
        &num_devices,
        physical_devices.data());
    
    if (enumerate_devices_result != VK_SUCCESS )
    {
        std::cout << "Error occurred during physical devices enumeration!" << std::endl;
        return;
    }
}

std::shared_ptr<scene::Renderer> RendererFactory::create()
{
    return std::make_shared<VkRenderer>();
}

} // namespace vk
