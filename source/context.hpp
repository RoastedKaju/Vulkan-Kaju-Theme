#pragma once

#include <string>
#include <vector>
#include <volk.h>
#include <vma/vk_mem_alloc.h>

class Context
{
public:
    static constexpr uint32_t cVulkanVersion{VK_API_VERSION_1_4};
    static constexpr uint32_t cMaxFramesInFlight{2};
    static constexpr VkFormat cSwapchainFormat{VK_FORMAT_B8G8R8A8_SRGB};
    static constexpr VkFormat cDepthFormat{VK_FORMAT_D32_SFLOAT};

    void init();

    void shutdown();

private:
    void createInstance();

    VkInstance mInstance;
};
