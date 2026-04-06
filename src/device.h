#pragma once

#include "common_types.h"

class Device
{
public:
    Device();
    ~Device();

    inline VkPhysicalDevice getPhysicalDevice() const { return physical_device; }
    inline VkDevice getDevice() const { return device; }
    inline VkQueue getGraphicsQueue() const { return graphics_queue; }
    inline uint32_t getGraphicsQueueFamily() const { return graphics_queue_family; }

    void createDevice(vkb::Instance &vkb_instance, VkSurfaceKHR surface);
    void deviceWaitIdle();
    void destroyDevice();

private:
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    uint32_t graphics_queue_family;
};