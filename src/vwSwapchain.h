#pragma once

#include "vwCommon.h"
#include "vwAllocator.h"

constexpr uint32_t frameOverlapCount = 2;

namespace vw
{
    class Instance;
    class Window;

    struct FrameData
    {
        VkCommandPool cmdPool;
        VkCommandBuffer cmdBuffer;
        VkSemaphore acquireSemaphore;
        VkFence renderFence;
        DeletionQueue frameDeletionQueue;
    };

    class Swapchain
    {
    public:
        void createSwapchain(Instance &instance, Window &window);
        void destroySwapchain(Instance &instance);

        void createSyncStructures(Instance &instance);
        void destroySyncStructures(Instance &instance);

        inline VkSwapchainKHR getSwapchain() const { return swapchain; }
        inline VkFormat getFormat() const { return format; }
        inline std::vector<VkImage> &getImages() { return images; }
        inline std::vector<VkImageView> &getViews() { return views; }
        inline std::vector<VkSemaphore> &getReleaseSemaphores() { return releaseSemaphores; }
        inline VkExtent2D getExtent() const { return extent; }
        inline FrameData &getCurrentFrameData() { return frames[frameNumber % frameOverlapCount]; }
        inline void incrementFrame() { frameNumber = (frameNumber + 1) % frameOverlapCount; }
        inline uint32_t getFrameNumber() const { return frameNumber; }

    private:
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkFormat format;
        std::vector<VkImage> images;
        std::vector<VkImageView> views;
        std::vector<VkSemaphore> releaseSemaphores;
        VkExtent2D extent;

        // sync data
        FrameData frames[frameOverlapCount];
        uint32_t frameNumber = 0;
    };
}