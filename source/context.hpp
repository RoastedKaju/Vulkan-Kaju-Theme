#pragma once

#include <string>
#include <vector>
#include <volk.h>
#include <vma/vk_mem_alloc.h>

struct GLFWwindow;

class Context
{
public:
    static constexpr uint32_t cVulkanVersion{VK_API_VERSION_1_4};
    static constexpr uint32_t cMaxFramesInFlight{2};
    static constexpr VkFormat cSwapchainFormat{VK_FORMAT_B8G8R8A8_SRGB};
    static constexpr VkFormat cDepthFormat{VK_FORMAT_D32_SFLOAT};

    void init(GLFWwindow *inWindow);

    void shutdown();

private:
    void createInstance();

    void createSurface();

    void findPhysicalDevice();

    void findGraphicsQueue();

    void createDevice();

    void initializeVMA();

    GLFWwindow *pWindow{nullptr};
    VkInstance mInstance{VK_NULL_HANDLE};
    VkSurfaceKHR mSurface{VK_NULL_HANDLE};
    VkPhysicalDevice mPhysicalDevice{VK_NULL_HANDLE};
    VkQueue mGraphicsQueue{VK_NULL_HANDLE};
    uint32_t mGraphicsQueueFamily{UINT32_MAX};
    VkDevice mDevice{VK_NULL_HANDLE};
    VmaAllocator mAllocator{VK_NULL_HANDLE};
};
