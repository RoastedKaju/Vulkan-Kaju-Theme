#pragma once

#include "vwCommon.h"

namespace vw
{
    class Window;
    
    class Instance
    {
    public:
        Instance() = default;
        ~Instance() = default;

        void createInstance();
        void destroyInstance();

        void createDevice(Window &window);
        void destroyDevice();

        inline VkInstance getInstance() const { return instance; }
        inline bool getInstanceStatus() const { return bInstanceIsValid; }

        inline VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
        inline VkDevice getDevice() const { return device; }
        inline VkQueue getGraphicsQueue() const { return graphicsQueue; }
        inline uint32_t getGraphicsQueueFamily() const { return graphicsQueueFamily; }

    private:
        VkInstance instance;
        vkb::Instance vkbInstance;
        VkDebugUtilsMessengerEXT debugUtilMessenger;
        bool bInstanceIsValid = false;

        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkQueue graphicsQueue;
        uint32_t graphicsQueueFamily;
    };
}