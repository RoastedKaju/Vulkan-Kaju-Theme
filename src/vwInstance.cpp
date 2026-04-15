#include "vwInstance.h"
#include "vwWindow.h"

void vw::Instance::createInstance()
{
    vkb::InstanceBuilder builder{};
    builder.request_validation_layers();
    builder.use_default_debug_messenger();
    builder.require_api_version(1, 3, 0);

    vkbInstance = builder.build().value();
    instance = vkbInstance.instance;
    debugUtilMessenger = vkbInstance.debug_messenger;

    std::cout << "Instance created\n";
}

void vw::Instance::destroyInstance()
{
    vkb::destroy_debug_utils_messenger(instance, debugUtilMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);

    std::cout << "Instance destroyed\n";
}

void vw::Instance::createDevice(Window &window)
{
    VkPhysicalDeviceVulkan13Features features13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    // select physical device
    vkb::PhysicalDeviceSelector selector{vkbInstance, window.getSurface()};
    selector.set_minimum_version(1, 3);
    selector.set_required_features_12(features12);
    selector.set_required_features_13(features13);
    selector.set_surface(window.getSurface());

    vkb::PhysicalDevice vkbPhysicalDevice = selector.select().value();

    vkb::DeviceBuilder builder{vkbPhysicalDevice};
    vkb::Device vkbDevice = builder.build().value();

    device = vkbDevice.device;
    physicalDevice = vkbPhysicalDevice.physical_device;

    // queues
    graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    std::cout << "Device created\n";
}

void vw::Instance::destroyDevice()
{
    vkDestroyDevice(device, nullptr);
    std::cout << "Device destroyed\n";
}
