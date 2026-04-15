#pragma once

#include "vwCommon.h"

namespace vw
{
    class Instance;
    class Window;

    class Swapchain
    {
    public:
        void createSwapchain(Instance &instance, Window &window);
        void destroySwapchain(Instance &instance);

        inline VkSwapchainKHR getSwapchain() const { return swapchain; }
        inline VkFormat getFormat() const { return format; }
        inline std::vector<VkImage> &getImages() { return images; }
        inline std::vector<VkImageView> &getViews() { return views; }
        inline std::vector<VkSemaphore> getReleaseSemaphores() { return releaseSemaphores; }
        inline VkExtent2D getExtent() const { return extent; }

    private:
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkFormat format;
        std::vector<VkImage> images;
        std::vector<VkImageView> views;
        std::vector<VkSemaphore> releaseSemaphores;
        VkExtent2D extent;
    };
}