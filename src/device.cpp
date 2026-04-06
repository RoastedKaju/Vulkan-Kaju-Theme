#include "device.h"

#include "instance.h"
#include "kaju_window.h"

Device::Device()
{
    std::cout << "Creating Device manager.\n";
}

Device::~Device()
{
    std::cout << "Destroying Device manager.\n";
}

void Device::createDevice(Instance &instance, KajuWindow &window)
{
    VkPhysicalDeviceVulkan13Features features_13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    features_13.dynamicRendering = true;
    features_13.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features_12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features_12.bufferDeviceAddress = true;
    features_12.descriptorIndexing = true;

    // Select a physical device
    vkb::PhysicalDeviceSelector selector{instance.getVkbInstance(), window.getSurface()};
    vkb::PhysicalDevice vkb_phyiscal_device = selector
                                                  .set_minimum_version(1, 3)
                                                  .set_required_features_13(features_13)
                                                  .set_required_features_12(features_12)
                                                  .set_surface(window.getSurface())
                                                  .select()
                                                  .value();

    vkb::DeviceBuilder builder{vkb_phyiscal_device};
    vkb::Device vkb_device = builder.build().value();

    device = vkb_device.device;
    physical_device = vkb_phyiscal_device.physical_device;

    // Graphics queue
    graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
    graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
}

void Device::deviceWaitIdle()
{
    vkDeviceWaitIdle(device);
}

void Device::destroyDevice()
{
    vkDestroyDevice(device, nullptr);
}