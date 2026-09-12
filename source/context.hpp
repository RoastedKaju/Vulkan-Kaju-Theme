#pragma once

#include <string>
#include <vector>
#include <volk.h>
#include <vma/vk_mem_alloc.h>
#include <shaderc/shaderc.hpp>

struct GLFWwindow;

class Context
{
public:
    static constexpr uint32_t cVulkanVersion{VK_API_VERSION_1_4};
    static constexpr uint32_t cMaxFramesInFlight{2};
    static constexpr VkFormat cSwapchainFormat{VK_FORMAT_B8G8R8A8_SRGB};
    static constexpr VkFormat cDepthFormat{VK_FORMAT_D32_SFLOAT};

    void init(GLFWwindow *inWindow);

    VkShaderModule createShaderModule(const std::string &fileName, shaderc_shader_kind kind) const;

    void shutdown();

private:
    void createInstance();

    void createSurface();

    void findPhysicalDevice();

    void findGraphicsQueue();

    void createDevice();

    void initializeVMA();

    void createSwapchain(uint32_t inWidth, uint32_t inHeight);

    void destroySwapchain();

    void createShaders();

    GLFWwindow *pWindow{nullptr};
    VkInstance mInstance{VK_NULL_HANDLE};
    VkSurfaceKHR mSurface{VK_NULL_HANDLE};
    VkPhysicalDevice mPhysicalDevice{VK_NULL_HANDLE};
    VkQueue mGraphicsQueue{VK_NULL_HANDLE};
    uint32_t mGraphicsQueueFamily{UINT32_MAX};
    VkDevice mDevice{VK_NULL_HANDLE};
    VmaAllocator mAllocator{VK_NULL_HANDLE};
    VkSwapchainKHR mSwapchain{VK_NULL_HANDLE};
    std::vector<VkImage> mSwapchainImages;
    std::vector<VkImageView> mSwapchainImageViews;
    std::vector<VkSemaphore> mRenderCompleteSemaphores;
    bool requireSwapchainRecreate = false;
    uint32_t mSwapchainWidth{0};
    uint32_t mSwapchainHeight{0};
    VkImage mDepthImage{VK_NULL_HANDLE};
    VkImageView mDepthImageView{VK_NULL_HANDLE};
    VmaAllocation mDepthImageAllocation{VK_NULL_HANDLE};
    VkShaderModule mVertShader{VK_NULL_HANDLE};
    VkShaderModule mFragShader{VK_NULL_HANDLE};
};
