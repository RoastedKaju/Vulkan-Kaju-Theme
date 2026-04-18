#include "vwSwapchain.h"
#include "vwInstance.h"
#include "vwWindow.h"
#include "vwUtils.h"

void vw::Swapchain::createSwapchain(Instance &instance, Window &window)
{
    format = VK_FORMAT_R8G8B8A8_UNORM;
    int width, height;
    glfwGetFramebufferSize(window.getWindow(), &width, &height);
    const VkExtent2D builderExtent = {(uint32_t)width, (uint32_t)height};

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

    // create release semaphores
    releaseSemaphores.resize(views.size(), VK_NULL_HANDLE);
    auto semaphoreInfo = utils::semaphoreCreateInfo();
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
    views.clear();
    releaseSemaphores.clear();

    std::cout << "Destroyed swapchain.\n";
}

void vw::Swapchain::createSyncStructures(Instance &instance)
{
    VkCommandPoolCreateInfo cmdPoolInfo = utils::cmdPoolCreateInfo(instance.getGraphicsQueueFamily(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkFenceCreateInfo fenceCreateInfo = utils::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = utils::semaphoreCreateInfo();
    for (int i = 0; i < frameOverlapCount; ++i)
    {
        // initialize command pool and command buffers
        VW_CHECK(vkCreateCommandPool(instance.getDevice(), &cmdPoolInfo, nullptr, &frames[i].cmdPool));
        VkCommandBufferAllocateInfo cmdAllocInfo = utils::cmdBufferAllocateInfo(frames[i].cmdPool, 1);
        VW_CHECK(vkAllocateCommandBuffers(instance.getDevice(), &cmdAllocInfo, &frames[i].cmdBuffer));
        // create acquire and render fences per frame
        VW_CHECK(vkCreateFence(instance.getDevice(), &fenceCreateInfo, nullptr, &frames[i].renderFence));
        VW_CHECK(vkCreateSemaphore(instance.getDevice(), &semaphoreCreateInfo, nullptr, &frames[i].acquireSemaphore));
    }
}

void vw::Swapchain::destroySyncStructures(Instance &instance)
{
    for (int i = 0; i < frameOverlapCount; ++i)
    {
        vkDestroyCommandPool(instance.getDevice(), frames[i].cmdPool, nullptr);
        vkDestroyFence(instance.getDevice(), frames[i].renderFence, nullptr);
        vkDestroySemaphore(instance.getDevice(), frames[i].acquireSemaphore, nullptr);
        frames[i].frameDeletionQueue.flush();
    }
}
