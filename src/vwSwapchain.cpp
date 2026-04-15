#include "vwSwapchain.h"
#include "vwInstance.h"
#include "vwWindow.h"
#include "vwUtils.h"

void vw::Swapchain::createSwapchain(Instance &instance, Window &window)
{
    format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkExtent2D builderExtent = window.getWindowExtent();

    vkb::SwapchainBuilder builder{instance.getPhysicalDevice(), instance.getDevice(), window.getSurface()};
    builder.set_desired_extent(builderExtent.width, builderExtent.height);
    builder.set_desired_format(VkSurfaceFormatKHR{.format = format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
    builder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR);
    builder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    builder.set_old_swapchain(swapchain);

    vkb::Swapchain vkbSwapchain = builder.build().value();

    // destroy old swapchain
    if (swapchain != VK_NULL_HANDLE)
    {
        std::cout << "Destroying old swapchain.\n";
        destroySwapchain(instance);
    }

    extent = vkbSwapchain.extent;
    swapchain = vkbSwapchain.swapchain;
    images = vkbSwapchain.get_images().value();
    views = vkbSwapchain.get_image_views().value();

    releaseSemaphores.resize(images.size(), VK_NULL_HANDLE);

    auto semaphoreInfo = utils::generateSemaphoreCreateInfo();
    for (auto &semaphore : releaseSemaphores)
    {
        VW_CHECK(vkCreateSemaphore(instance.getDevice(), &semaphoreInfo, nullptr, &semaphore));
    }

    std::cout << "Created swapchain.\n";
}

void vw::Swapchain::destroySwapchain(Instance &instance)
{
    vkDestroySwapchainKHR(instance.getDevice(), swapchain, nullptr);

    for (int i = 0; i < views.size(); ++i)
    {
        vkDestroyImageView(instance.getDevice(), views[i], nullptr);
    }
    for (auto &semaphore : releaseSemaphores)
    {
        vkDestroySemaphore(instance.getDevice(), semaphore, nullptr);
    }
    releaseSemaphores.clear();

    std::cout << "Destroyed swapchain.\n";
}
