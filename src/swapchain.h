#pragma once

#include "common_types.h"

class KajuWindow;
class Device;

class Swapchain
{
public:
    Swapchain() = default;
    ~Swapchain() = default;

    inline VkSwapchainKHR getSwapchain() const { return swapchain; }
    inline std::vector<VkImage> &getImages() { return images; }
    inline std::vector<VkImageView> &getImageViews() { return views; }
    inline std::vector<VkSemaphore> &getRenderCompleteSemaphores() { return render_complete_semaphores; }
    inline const VkFormat &getFormat() const { return format; }
    inline VkExtent2D getExtent() const { return extent; }

    void createSwapchain(Device &device, KajuWindow &window);
    void destroySwapchain(Device &device);

private:
    VkSwapchainKHR swapchain;
    VkFormat format;
    std::vector<VkImage> images;
    std::vector<VkImageView> views;
    std::vector<VkSemaphore> render_complete_semaphores;
    // Reference to fences owned by frame manager
    std::vector<VkFence> images_in_flight;
    VkExtent2D extent;
};