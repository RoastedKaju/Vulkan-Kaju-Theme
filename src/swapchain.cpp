#include "swapchain.h"

#include "device.h"
#include "kaju_window.h"

void Swapchain::createSwapchain(Device &device, KajuWindow &window)
{
    vkb::SwapchainBuilder builder{device.getPhysicalDevice(), device.getDevice(), window.getSurface()};

    format = VK_FORMAT_R8G8B8A8_UNORM;

    int width = 0, height = 0;
    glfwGetFramebufferSize(window.getWindow(), &width, &height);

    vkb::Swapchain vkb_swapchain = builder
                                       .set_desired_format(VkSurfaceFormatKHR{.format = format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                                       .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                       .set_desired_extent(width, height)
                                       .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                       .build()
                                       .value();

    extent = vkb_swapchain.extent;
    swapchain = vkb_swapchain.swapchain;
    images = vkb_swapchain.get_images().value();
    views = vkb_swapchain.get_image_views().value();

    // Fence for swapchain
    images_in_flight.resize(images.size(), VK_NULL_HANDLE);
    render_complete_semaphores.resize(images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = nullptr;
    semaphore_create_info.flags = 0;
    // Create semaphores for render complete signal
    for (auto &semaphore : render_complete_semaphores)
    {
        if (vkCreateSemaphore(device.getDevice(), &semaphore_create_info, nullptr, &semaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image render semaphore.");
        }
    }
}

void Swapchain::destroySwapchain(VkDevice device)
{
    vkDestroySwapchainKHR(device, swapchain, nullptr);

    // Destroy swapchain resources
    for (int i = 0; i < views.size(); ++i)
    {
        vkDestroyImageView(device, views[i], nullptr);
    }

    for (auto semaphore : render_complete_semaphores)
    {
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    render_complete_semaphores.clear();
    // No need to destroy these fences as these are owned by frame-manager
    images_in_flight.clear();
}
